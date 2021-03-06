#include "ClubRepository.h"
#include "ClubRepositoryInternal.h"

#include <QFile>
#include <QByteArray>
#include <QSqlResult>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


#include "DbUtils.h"
#include "NameFormatUtils.h"

using namespace sqlx;

static struct {
    int schemaVersion;
    QString sqlFile;
} schemas[] = {
        {1, QStringLiteral(":/sql/db_v1.sql")},
        {2, QStringLiteral(":/sql/db_v2.sql")},
        {3, QStringLiteral(":/sql/db_v3.sql")},
        {4, QStringLiteral(":/sql/db_v4.sql")},
        {5, QStringLiteral(":/sql/db_v5.sql")},
};

static const SettingKey skClubName = QStringLiteral("club_name");
static const SettingKey skLevelMin = QStringLiteral("level_min");
static const SettingKey skLevelMax = QStringLiteral("level_max");

static const unsigned defaultLevelMin = 1;
static const unsigned defaultLevelMax = 5;

class SQLTransaction {
    QSqlDatabase &db_;
    bool rollback_ = false;

public:
    explicit SQLTransaction(QSqlDatabase &db) : db_(db) {
        db.transaction();
    }

    void setError() {
        rollback_ = true;
    }

    ~SQLTransaction() {
        if (rollback_) db_.rollback();
        else db_.commit();
    }
};

struct ClubRepository::Impl {
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
};

ClubRepository::ClubRepository(QObject *parent, Impl *d)
        : QObject(parent), d(d) {}

ClubRepository::~ClubRepository() {
    d->db.close();
    delete d;
}

ClubRepository *ClubRepository::open(QObject *parent, const QString &path) {
    std::unique_ptr<Impl> d(new Impl);
    d->db.setDatabaseName(path);
    if (!d->db.open() || !d->db.isValid()) {
        qCritical().noquote() << "Error opening: " << path << " : " << d->db.lastError();
        return nullptr;
    }

    int currSchemaVersion = 0;

    SQLTransaction tx(d->db);

    bool hasSettingsTable = DbUtils::queryFirst<int>(
            d->db,
            QStringLiteral("select count(*) from sqlite_master where type = 'table' and name = 'settings'")).orDefault(
            0) > 0;

    if (hasSettingsTable) {
        if (auto result = DbUtils::queryFirst<int>(
                d->db,
                QStringLiteral("select cast(value as integer) from settings where name = 'schema_version'"))) {
            currSchemaVersion = *result;
        }
    }

    QSqlQuery q(d->db);

    for (const auto &schema : schemas) {
        if (schema.schemaVersion > currSchemaVersion) {
            QFile schemaFile(schema.sqlFile);
            if (!schemaFile.open(QIODevice::ReadOnly)) {
                tx.setError();
                qCritical() << "Unable to open file: " << schema.sqlFile;
                return nullptr;
            }

            auto sqls = QString::fromUtf8(schemaFile.readAll()).split(QStringLiteral("---"));
            for (auto sql : sqls) {
                sql = sql.trimmed();
                if (sql.isEmpty()) continue;
                if (!q.exec(sql)) {
                    tx.setError();
                    auto err = d->db.lastError();
                    qCritical() << "Error executing sql " << sql << ":" << err;
                    return nullptr;
                }
            }
            qDebug() << "Migrated to schema version " << schema.schemaVersion;
            currSchemaVersion = schema.schemaVersion;
        }
    }

    if (currSchemaVersion != schemas[sizeof(schemas) / sizeof(schemas[0]) - 1].schemaVersion) {
        qWarning() << "Unable to migrate to target schema version";
        return nullptr;
    }

    return new ClubRepository(parent, d.release());
}

std::optional<SessionId> ClubRepository::getLastSession() const {
    return DbUtils::queryFirst<SessionId>(d->db, QStringLiteral(
            "select id from sessions order by startTime desc, id desc limit 1")).toOptional();
}

std::optional<SessionData>
ClubRepository::createSession(unsigned fee, const QString &place, const QString &announcement,
                              unsigned numPlayersPerCourt,
                              const QVector<CourtConfiguration> &courts) {
    if (courts.isEmpty() || numPlayersPerCourt == 0) return std::nullopt;

    SQLTransaction trans(d->db);
    auto sessionId = DbUtils::insert<SessionId>(
            d->db,
            QStringLiteral("insert into sessions (fee, place, announcement, numPlayersPerCourt) values (?, ?, ?, ?)"),
            {fee, place, announcement, numPlayersPerCourt});
    if (!sessionId) {
        trans.setError();
        return std::nullopt;
    }

    for (const auto &court : courts) {
        auto courtResult = DbUtils::update(
                d->db,
                QStringLiteral(
                        "insert into courts (sessionId, name, sortOrder) values (?, ?, ?)"),
                {*sessionId, court.name, court.sortOrder});

        if (!courtResult) {
            trans.setError();
            return std::nullopt;
        }
    }

    if (auto data = getSession(*sessionId)) {
        emit this->sessionChanged(*sessionId);
        return data;
    }

    trans.setError();
    return std::nullopt;
}

QVector<GameAllocation> ClubRepository::getPastAllocations(SessionId id) const {
    return DbUtils::queryList<GameAllocation>(
            d->db,
            QStringLiteral("select GA.gameId, GA.courtId, P.memberId, GA.quality from game_allocations GA "
                           "inner join players P on P.id = GA.playerId "
                           "where GA.gameId in ( "
                           "select id from games where sessionId = ?)"),
            {id}).orDefault();
}

MemberGameStats ClubRepository::getMemberGameStats(MemberId memberId, SessionId sessionId) const {
    QVector<MemberGameStats::PastGame> pastGames;
    size_t numGames = 0;


    DbUtils::queryRawStream(
            d->db, QStringLiteral(
                    "select G.id as gameId, C.id as courtId, C.name as courtName, cast(strftime('%s', G.startTime) as integer) as startTime, GA.quality, M.* from game_allocations GA "
                    "inner join games G on G.id = GA.gameId "
                    "inner join players P on P.id = GA.playerId "
                    "inner join courts C on C.id = GA.courtId "
                    "inner join normalized_members M on M.id = P.memberId "
                    "where P.sessionId = ? "
                    "order by G.startTime desc, G.id, C.id, M.firstName, M.lastName"),
            {sessionId},
            [&](const QSqlRecord &record) -> bool {
                auto gameId = record.value(QStringLiteral("gameId")).value<GameId>();
                auto courtId = record.value(QStringLiteral("courtId")).value<GameId>();

                if (numGames == 0 || pastGames.last().gameId != gameId) {
                    numGames++;
                }

                if (pastGames.isEmpty() ||
                    (pastGames.last().gameId != gameId || pastGames.last().courtId != courtId)) {
                    pastGames.push_back({
                                                gameId, courtId, record.value(QStringLiteral("courtName")).toString(),
                                                QDateTime::fromSecsSinceEpoch(
                                                        record.value(QStringLiteral("startTime")).toLongLong()),
                                                record.value(QStringLiteral("quality")).toInt()
                                        });
                }

                auto &pastGame = pastGames.last();
                pastGame.players.push_back(BaseMember());
                if (!DbUtils::readFrom(pastGame.players.last(), record)) {
                    qWarning() << "Unable to read member for stats";
                }

                return true;
            });

    MemberGameStats gameStats = {numGames};

    for (const auto &game : pastGames) {
        for (const auto &player : game.players) {
            if (player.id == memberId) {
                gameStats.numGamesOff--;
                gameStats.pastGames.append(game);
                break;
            }
        }
    }

    return gameStats;
}


std::optional<GameId> ClubRepository::createGame(SessionId sessionId,
                                                 const QVector<GameAllocation> &allocations,
                                                 qlonglong durationSeconds) {
    if (allocations.empty()) return std::nullopt;

    SQLTransaction tx(d->db);

    auto gameId = DbUtils::insert<GameId>(
            d->db,
            QStringLiteral("insert into games (sessionId, durationSeconds) values (?, ?)"),
            {sessionId, durationSeconds});

    if (!gameId) {
        tx.setError();
        return std::nullopt;
    }

    for (const auto &ga : allocations) {
        auto insertResult = DbUtils::update(
                d->db,
                QStringLiteral("insert into game_allocations (gameId, courtId, playerId, quality) values (?, ?, "
                               "(select P.id from players P where P.memberId = ? and P.sessionId = ?), "
                               "?)"),
                {*gameId, ga.courtId, ga.memberId, sessionId, ga.quality});

        if (!insertResult) {
            tx.setError();
            return std::nullopt;
        }
    }

    emit this->sessionChanged(sessionId);
    return *gameId;
}

static void sanitizeMemberNames(QString &firstName, QString &lastName) {
    firstName = firstName.trimmed();
    lastName = lastName.trimmed();
}

std::optional<BaseMember>
ClubRepository::createMember(QString firstName,
                             QString lastName,
                             Member::Gender gender,
                             int level,
                             QString phone,
                             QString email) {
    sanitizeMemberNames(firstName, lastName);
    if (firstName.isEmpty() || lastName.isEmpty()) return std::nullopt;

    SQLTransaction tx(d->db);

    auto memberId = DbUtils::insert<MemberId>(
            d->db,
            QStringLiteral(
                    "insert into members (firstName, lastName, gender, level, registerDate, phone, email) values (?, ?, ?, ?, ?, ?, ?)"),
            {firstName, lastName, enumToString(gender).toLower(), level, QDateTime::currentSecsSinceEpoch(),
             phone.trimmed(), email.trimmed()});

    if (!memberId) {
        qWarning() << "Error inserting member";
        tx.setError();
        return std::nullopt;
    }

    emit memberChanged();

    return getMember(*memberId);
}

static std::pair<QString, QVector<QVariant>> constructFindMembersSql(const MemberSearchFilter &filter,
                                                                     const QString &extraWhere = QString(),
                                                                     const QVector<QVariant> &extraWhereArgs = {}) {
    QString sql;
    QVector<QVariant> args;
    if (std::get_if<AllMembers>(&filter)) {
        sql += QStringLiteral("select * from normalized_members where 1 %1 order by firstName, lastName").arg(
                extraWhere);
    } else if (auto checkedIn = std::get_if<CheckedIn>(&filter)) {
        if (checkedIn->paused == true) {
            sql += QStringLiteral(
                    "select * from paused_members where sessionId = ? %1 order by firstName, lastName").arg(extraWhere);
        } else if (checkedIn->paused == false) {
            sql += QStringLiteral(
                    "select * from checked_in_non_paused_members where sessionId = ? %1 order by firstName, lastName").arg(
                    extraWhere);
        } else {
            sql += QStringLiteral(
                    "select * from session_members where status != ? and sessionId = ? %1 order by firstName, lastName").arg(
                    extraWhere);
            args.push_back(enumToString(Member::CheckedOut));
        }

        args += checkedIn->sessionId;
    } else if (auto nonCheckedIn = std::get_if<NonCheckedIn>(&filter)) {
        sql += QStringLiteral(
                "select * from unchecked_in_members where sessionId = ? %1 order by firstName, lastName").arg(
                extraWhere);
        args += nonCheckedIn->sessionId;
    } else if (auto allSession = std::get_if<AllSession>(&filter)) {
        sql += QStringLiteral("select * from session_members where sessionId = ? %1 order by firstName, lastName").arg(
                extraWhere);
        args += allSession->sessionId;
    }

    args += extraWhereArgs;

    return std::make_pair(sql, args);
}

QVector<Member> ClubRepository::findMember(MemberSearchFilter filter, const QString &needle) const {
    QString extraWhere;
    QVector<QVariant> extraWhereArgs;

    auto trimmed = needle.trimmed();
    if (!trimmed.isEmpty()) {
        extraWhere = QStringLiteral(" and (firstName like ?)");
        extraWhereArgs += QStringLiteral("%1%%").arg(trimmed);
    }

    auto[sql, args] = constructFindMembersSql(filter, extraWhere, extraWhereArgs);
    return DbUtils::queryList<Member>(d->db, sql, args).orDefault();
}

QVector<Member> ClubRepository::getMembers(MemberSearchFilter filter) const {
    auto[sql, args] = constructFindMembersSql(filter);
    auto members = DbUtils::queryList<Member>(d->db, sql, args).orDefault();
    formatMemberDisplayNames(members);
    return members;
}

bool ClubRepository::checkIn(SessionId sessionId, MemberId memberId, bool paid) {
    auto rc = DbUtils::update(
            d->db,
            QStringLiteral(
                    "insert or replace into players (sessionId, memberId, paid, checkInTime, checkOutTime) values (?, ?, ?, current_timestamp, null)"),
            {sessionId, memberId, paid}).orDefault(0) > 0;
    if (rc) {
        emit this->sessionChanged(sessionId);
        emit memberChanged();
    }
    return rc;
}

bool ClubRepository::checkOut(SessionId sessionId, MemberId memberId) {
    auto rc = DbUtils::update(
            d->db,
            QStringLiteral("update players set checkOutTime = current_timestamp where sessionId = ? and memberId = ?"),
            {sessionId, memberId}).orDefault(0) > 0;
    if (rc) {
        emit this->sessionChanged(sessionId);
        emit memberChanged();
    }
    return rc;
}

std::optional<GameInfo> ClubRepository::getLastGameInfo(SessionId sessionId) const {
    auto gameResult = DbUtils::queryFirst<GameInfo>(
            d->db,
            QStringLiteral(
                    "select id, cast(strftime('%s',startTime) as integer) as startTime, durationSeconds from games "
                    "where sessionId = ? "
                    "order by startTime desc, id desc limit 1"),
            {sessionId});

    if (!gameResult) return std::nullopt;

    auto onMembers = DbUtils::queryList<GameAllocationMember>(
            d->db,
            QStringLiteral("select M.*, "
                           "C.id as courtId, C.name as courtName, GA.quality as courtQuality from game_allocations GA "
                           "inner join games G on G.id = GA.gameId "
                           "inner join players P on P.memberId = M.id and P.id = GA.playerId "
                           "inner join session_members M on M.id = P.memberId and M.sessionId = P.sessionId "
                           "inner join courts C on C.id = GA.courtId "
                           "where G.id = ? "
                           "order by C.sortOrder"),
            {gameResult->id});

    if (!onMembers) return std::nullopt;

    const auto allPlayers = getMembers(AllSession{sessionId});
    formatMemberDisplayNames(*onMembers, allPlayers);

    for (auto &member : *onMembers) {
        if (gameResult->courts.isEmpty() || gameResult->courts.last().courtId != member.courtId) {
            gameResult->courts.append(CourtPlayers{member.courtId, member.courtName, member.courtQuality});
        }

        gameResult->courts.last().players.append(member);
    }

    QSet<MemberId> onMemberIds;
    for (const auto &m : *onMembers) {
        onMemberIds.insert(m.id);
    }
    for (const auto &item : allPlayers) {
        if (!onMemberIds.contains(item.id) && item.status != Member::CheckedOut) {
            gameResult->waiting.append(item);
        }
    }
    formatMemberDisplayNames(gameResult->waiting, allPlayers);

    return *gameResult;
}

std::optional<QString> ClubRepository::getSetting(const SettingKey &key) const {
    return DbUtils::queryFirst<QString>(
            d->db, QStringLiteral("select value from settings where name = ?"), {key})
            .toOptional();
}

bool ClubRepository::saveSetting(const SettingKey &key, const QVariant &value) {
    return DbUtils::update(
            d->db,
            QStringLiteral("insert or replace into settings (name, value) values (?, ?)"),
            {key, value}).orDefault(0) > 0;
}

bool ClubRepository::removeSetting(const SettingKey &key) {
    return DbUtils::update(
            d->db,
            QStringLiteral("delete from settings where name = ?"),
            {key}).orDefault(0) > 0;
}

std::optional<BaseMember> ClubRepository::getMember(MemberId id) const {
    auto[sql, args] = constructFindMembersSql(AllMembers{},
                                              QStringLiteral(" and id = ?"),
                                              {id});

    return DbUtils::queryFirst<BaseMember>(d->db, sql, args).toOptional();
}

bool ClubRepository::withdrawLastGame(SessionId sessionId) {
    auto rc = DbUtils::update(d->db,
                              QStringLiteral(
                                      "delete from games where id = (select id from games where sessionId = ? order by startTime desc limit 1)"),
                              {sessionId}).orDefault(0) > 0;
    if (rc) {
        emit this->sessionChanged(sessionId);
    }
    return rc;
}

std::optional<SessionData> ClubRepository::getSession(SessionId sessionId) const {
    auto session = DbUtils::queryFirst<Session>(d->db, QStringLiteral("select * from sessions where id = ?"),
                                                {sessionId});

    if (!session) return std::nullopt;

    auto courts = DbUtils::queryList<Court>(d->db, QStringLiteral("select * from courts where sessionId = ?"),
                                            {sessionId});
    if (!courts) return std::nullopt;

    return SessionData{*session, *courts};
}

std::optional<MemberId> ClubRepository::findMemberBy(QString firstName, QString lastName) {
    sanitizeMemberNames(firstName, lastName);
    return DbUtils::queryFirst<MemberId>(
            d->db,
            QStringLiteral("select id from members where firstName = ? collate nocase and lastName = ? collate nocase"),
            {firstName, lastName}).toOptional();
}

bool ClubRepository::saveMember(const BaseMember &m) {
    if (DbUtils::update(
            d->db,
            QStringLiteral("update members set (firstName, lastName, gender, level, email, phone)"
                           " = (?, ?, ?, ?, ?, ?) where id = ?"),
            {m.firstName, m.lastName, enumToString(m.gender).toLower(), m.level, m.email.trimmed(), m.phone.trimmed(),
             m.id})
                .orDefault(0) > 0) {
        emit memberChanged();
        return true;
    }

    return false;
}

bool ClubRepository::setPaused(SessionId sessionId, MemberId memberId, bool paused) {
    if (DbUtils::update(d->db,
                        QStringLiteral("update players set paused = ? where sessionId = ? and memberId = ?"),
                        {paused, sessionId, memberId}).orDefault(0) > 0) {
        emit this->sessionChanged(sessionId);
        emit memberChanged();
        return true;
    }
    return false;
}

bool ClubRepository::setPaid(SessionId sessionId, MemberId memberId, bool paid) {
    if (DbUtils::update(d->db,
                        QStringLiteral("update players set paid = ? where sessionId = ? and memberId = ?"),
                        {paid, sessionId, memberId}).orDefault(0) > 0) {
        emit this->sessionChanged(sessionId);
        emit memberChanged();
        return true;
    }
    return false;
}


QString ClubRepository::getClubName() const {
    return getSettingValue<QString>(skClubName).value_or(QString());
}

bool ClubRepository::saveClubName(const QString &name) {
    return saveSetting(skClubName, name);
}

LevelRange ClubRepository::getLevelRange() const {
    SQLTransaction tx(d->db);
    return {
            getSettingValue<unsigned>(skLevelMin).value_or(defaultLevelMin),
            getSettingValue<unsigned>(skLevelMax).value_or(defaultLevelMax)
    };
}

bool ClubRepository::saveClubInfo(const QString &name, LevelRange range) {
    if (!range.isValid()) {
        return false;
    }

    SQLTransaction tx(d->db);
    if (!saveSetting(skLevelMin, range.min) || !saveSetting(skLevelMax, range.max)) {
        tx.setError();
        return false;
    }

    if (!saveSetting(skClubName, name)) {
        tx.setError();
        return false;
    }

    return true;
}

size_t
ClubRepository::importMembers(std::function<bool(BaseMember &)> memberSupplier, QVector<BaseMember> *failMembers) {
    SQLTransaction tx(d->db);

    size_t success = 0;
    Member member;
    while (memberSupplier(member)) {
        sanitizeMemberNames(member.firstName, member.lastName);
        if (member.firstName.isEmpty() || member.lastName.isEmpty()) {
            if (failMembers) failMembers->push_back(member);
            continue;
        }

        auto memberId = DbUtils::insert<MemberId>(
                d->db,
                QStringLiteral(
                        "insert or replace into members (firstName, lastName, gender, level) values (?, ?, ?, ?)"),
                {member.firstName, member.lastName, enumToString(member.gender).toLower(), member.level});

        if (!memberId) {
            if (failMembers) failMembers->push_back(member);
        } else {
            success++;
        }
    }

    emit memberChanged();
    return success;
}

QVector<PaymentRecord> ClubRepository::getPaymentRecords(const QSet<SessionId> &sessionIds) const {
    auto sql = QStringLiteral("select M.id as memberId, "
                              "M.firstName as memberFirstName, "
                              "M.lastName as memberLastName, "
                              "M.paid, M.sessionId, (select cast(strftime('%s', startTime) as INTEGER) from sessions where id = M.sessionId) as sessionStartTime from session_members M "
                              "where M.sessionId in (");
    for (const auto &id : sessionIds) {
        sql += QString::number(id);
        sql += QStringLiteral(",");
    }

    sql.replace(sql.length() - 1, 1, QStringLiteral(")"));

    return DbUtils::queryList<PaymentRecord>(d->db, sql).orDefault();
}

QVector<Session> ClubRepository::getAllSessions(std::optional<size_t> limit) {
    auto sql = QStringLiteral("select * from sessions order by startTime desc, id desc");
    QVector<QVariant> args;
    if (limit) {
        sql += QStringLiteral(" limit ?");
        args.push_back(QVariant::fromValue(*limit));
    }

    return DbUtils::queryList<Session>(d->db, sql, args).orDefault();
}


