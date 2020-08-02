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

static struct {
    int schemaVersion;
    QString sqlFile;
} schemas[] = {
        {1, QStringLiteral(":/sql/db_v1.sql")},
        {2, QStringLiteral(":/sql/db_v2.sql")},
};

static const SettingKey skClubName = QStringLiteral("club_name");
static const SettingKey skLevelMin = QStringLiteral("level_min");
static const SettingKey skLevelMax = QStringLiteral("level_max");

static const unsigned int defaultLevelMin = 1;
static const unsigned int defaultLevelMax = 4;

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
    QSqlDatabase db;
};

ClubRepository::ClubRepository(QObject *parent, const QSqlDatabase &db)
        : QObject(parent), d(new Impl{db}) {}

ClubRepository::~ClubRepository() {
    delete d;
}

ClubRepository *ClubRepository::open(QObject *parent, const QString &path) {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(path);
    db.open();
    if (!db.isValid()) {
        qCritical().noquote() << "Error opening: " << path << " : " << db.lastError();
        return nullptr;
    }

    int currSchemaVersion = 0;

    SQLTransaction tx(db);

    auto result = DbUtils::queryFirst<Setting>(db,
                                               QStringLiteral("select * from settings where name = 'schema_version'"));

    if (result) {
        currSchemaVersion = result->value.toInt();
    }

    QSqlQuery q(db);

    for (const auto &schema : schemas) {
        if (schema.schemaVersion > currSchemaVersion) {
            qDebug() << "Migrating to schema version " << schema.schemaVersion;
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
                    auto err = db.lastError();
                    qCritical() << "Error executing sql " << sql << ":" << err;
                    return nullptr;
                }
            }
            qDebug() << "Migrated to schema version " << schema.schemaVersion;
        }
    }

    return new ClubRepository(parent, db);
}

std::optional<SessionId> ClubRepository::getLastSession() const {
    auto result = DbUtils::queryFirst<SessionId>(d->db, QStringLiteral(
            "select id from sessions order by startTime desc limit 1"));
    if (result) {
        return *result;
    }
    return std::nullopt;
}

std::optional<SessionData>
ClubRepository::createSession(int fee, const QString &place, const QString &announcement, int numPlayersPerCourt,
                              const QVector<CourtConfiguration> &courts) {
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

    if (auto data = getSession(sessionId)) {
        return data;
    }

    trans.setError();
    return std::nullopt;
}

QVector<GameAllocation> ClubRepository::getPastAllocations(SessionId id, std::optional<size_t> numGames) const {
    auto sql = QStringLiteral("select GA.gameId, GA.courtId, P.memberId from game_allocations GA "
                              "inner join players P on P.id = GA.playerId "
                              "where GA.gameId in ( "
                              "select id from games where sessionId = ? ");

    if (numGames) {
        sql += QStringLiteral("order by startTime desc limit %1 ").arg(*numGames);
    }

    sql += QStringLiteral(")");

    return DbUtils::queryList<GameAllocation>(d->db, sql, {id}).orDefault();
}

std::optional<GameId> ClubRepository::createGame(SessionId sessionId,
                                                 nonstd::span<const GameAllocation> allocations,
                                                 qlonglong durationSeconds) {
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
                               "(select P.id from players P where P.memberId = ?), "
                               "?)"),
                {*gameId, ga.courtId, ga.memberId, ga.quality});

        if (!insertResult) {
            tx.setError();
            return std::nullopt;
        }
    }

    emit this->sessionChanged(sessionId);
    return gameId;
}

std::optional<Member>
ClubRepository::createMember(const QString &fistName,
                             const QString &lastName,
                             const Member::Gender &gender,
                             int level) {
    SQLTransaction tx(d->db);

    auto memberId = DbUtils::insert<MemberId>(
            d->db,
            QStringLiteral("insert into members (firstName, lastName, gender, level) values (?, ?, ?, ?)"),
            {fistName, lastName, enumToString(gender), level});

    if (!memberId) {
        qWarning() << "Error inserting member";
        tx.setError();
        return std::nullopt;
    }

    return getMember(*memberId);
}

static std::pair<QString, QVector<QVariant>> constructFindMembersSql(const MemberSearchFilter &filter) {
    QString sql;
    QVector<QVariant> args;
    if (std::get_if<AllMembers>(&filter)) {
        sql += QStringLiteral("select * from members M where 1");
    } else if (auto checkedIn = std::get_if<CheckedIn>(&filter)) {
        sql += QStringLiteral("select M.*, P.paid as paid, "
                              " (case "
                              "     when P.paused then %1 "
                              "     else %2 "
                              " end) as status "
                              "from members M "
                              "inner join players P on P.memberId = M.id "
                              "where P.checkOutTime is null "
                              " and P.sessionId = ? ").arg(QString::number(Member::CheckedInPaused),
                                                           QString::number(Member::CheckedIn));
        args.push_back(checkedIn->sessionId);

        if (checkedIn->paused) {
            sql += QStringLiteral(" and P.paused = ?");
            args.push_back(*checkedIn->paused);
        }
    } else if (auto nonCheckedIn = std::get_if<NonCheckedIn>(&filter)) {
        sql += QStringLiteral("select M.*, %1 as status from members M "
                              "  where id not in (select memberId from players where sessionId = ? and checkOutTime is null)")
                .arg(Member::NotCheckedIn);
        args.push_back(nonCheckedIn->sessionId);
    } else if (auto allSession = std::get_if<AllSession>(&filter)) {
        sql += QStringLiteral("select M.*, "
                              " (case "
                              "     when (P.checkoutTime is not null) then %1 "
                              "     when P.paused then %2 "
                              "     else %3 "
                              " end) as status, "
                              "P.paid as paid "
                              "from members M "
                              "inner join players P on P.memberId = M.id "
                              "where P.sessionId = ?").arg(
                QString::number(Member::CheckedOut),
                QString::number(Member::CheckedInPaused),
                QString::number(Member::CheckedIn));
        args.push_back(allSession->sessionId);
    }

    return std::make_pair(sql, args);
}

QVector<Member> ClubRepository::findMember(MemberSearchFilter filter, const QString &needle) const {
    auto[sql, args] = constructFindMembersSql(filter);
    auto trimmed = needle;
    if (!trimmed.isEmpty()) {
        sql += QStringLiteral(" and (M.firstName like ?)");
        auto realNeedle = QStringLiteral("%1%%").arg(trimmed);
        args.push_back(realNeedle);
    }

    return DbUtils::queryList<Member>(d->db, sql, args).orDefault();
}

QVector<Member> ClubRepository::getMembers(MemberSearchFilter filter) const {
    auto[sql, args] = constructFindMembersSql(filter);
    auto members = DbUtils::queryList<Member>(d->db, sql, args).orDefault();
    formatMemberDisplayNames(members);
    return members;
}

bool ClubRepository::checkIn(MemberId memberId, SessionId sessionId, bool paid) {
    auto rc = DbUtils::update(
            d->db,
            QStringLiteral("insert or replace into players (sessionId, memberId, paid, checkInTime, checkOutTime) values (?, ?, ?, current_timestamp, null)"),
            {sessionId, memberId, paid}).orDefault(0) > 0;
    if (rc) {
        emit this->sessionChanged(sessionId);
    }
    return rc;
}

bool ClubRepository::checkOut(SessionId sessionId, MemberId memberId) {
    auto rc = DbUtils::update(
            d->db,
            QStringLiteral("update players set checkOutTime = current_timestamp where sessionId = ? and memberId = ?"),
            {sessionId, memberId}).orDefault(0) > 0;
    if (rc) emit this->sessionChanged(sessionId);
    return rc;
}

std::optional<GameInfo> ClubRepository::getLastGameInfo(SessionId sessionId) const {
    auto gameResult = DbUtils::queryFirst<GameInfo>(
            d->db,
            QStringLiteral(
                    "select id, cast(strftime('%s',startTime) as integer) as startTime, durationSeconds from games "
                    "where sessionId = ? "
                    "order by startTime desc limit 1"),
            {sessionId});

    if (!gameResult) return std::nullopt;

    auto onMembers = DbUtils::queryList<GameAllocationMember>(
            d->db,
            QStringLiteral("select M.*, P.paid as paid, "
                           "(case when (P.checkOutTime is not null) then %1 "
                           "when P.paused then %2 "
                           "else %3 "
                           "end) as status, C.id as courtId, C.name as courtName from game_allocations GA "
                           "inner join games G on G.id = GA.gameId "
                           "inner join players P on P.memberId = M.id and P.id = GA.playerId "
                           "inner join members M on M.id = P.memberId "
                           "inner join courts C on C.id = GA.courtId "
                           "where G.id = ? "
                           "order by C.sortOrder").arg(
                    QString::number(Member::CheckedOut),
                    QString::number(Member::CheckedInPaused),
                    QString::number(Member::CheckedIn)),
            {gameResult->id, sessionId});

    if (!onMembers) return std::nullopt;

    const auto allPlayers = getMembers(AllSession{sessionId});
    formatMemberDisplayNames(*onMembers, allPlayers);

    for (auto &member : *onMembers) {
        if (gameResult->courts.isEmpty() || gameResult->courts.last().courtId != member.courtId) {
            gameResult->courts.append(CourtPlayers{member.courtId, member.courtName});
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
    return DbUtils::insert<Setting>(
            d->db,
            QStringLiteral("delete from settings where name = ?"),
            {key});
}

std::optional<Member> ClubRepository::getMember(MemberId id) const {
    auto[sql, args] = constructFindMembersSql(AllMembers{});
    sql += QStringLiteral(" and M.id = ?");
    args.push_back(id);

    return DbUtils::queryFirst<Member>(d->db, sql, args).toOptional();
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


    std::optional<SessionData> data;
    data.emplace().session = *session;
    data.value().courts = *courts;
    return data;
}

std::optional<MemberId> ClubRepository::findMemberBy(const QString &firstName, const QString &lastName) {
    return DbUtils::queryFirst<MemberId>(
            d->db,
            QStringLiteral("select id from members where firstName = ? and lastName = ?"),
            {firstName, lastName}).toOptional();
}

bool ClubRepository::saveMember(const Member &m) {
    return DbUtils::update(
            d->db,
            QStringLiteral("update members set (firstName, lastName, gender, level, email, phone)"
                           " = (?, ?, ?, ?, ?, ?) where id = ?"),
            {m.firstName, m.lastName, enumToString(m.gender), m.level, m.email, m.phone, m.id})
                   .orDefault(0) > 0;

}

bool ClubRepository::setPaused(SessionId sessionId, MemberId memberId, bool paused) {
    if (DbUtils::update(d->db,
                        QStringLiteral("update players set paused = ? where sessionId = ? and memberId = ?"),
                        {paused, sessionId, memberId}).orDefault(0) > 0) {
        emit this->sessionChanged(sessionId);
        return true;
    }
    return false;
}

bool ClubRepository::setPaid(SessionId sessionId, MemberId memberId, bool paid) {
    if (auto rc = DbUtils::update(d->db,
                                  QStringLiteral("update players set paid = ? where sessionId = ? and memberId = ?"),
                                  {paid, sessionId, memberId}).orDefault(0) > 0) {
        emit this->sessionChanged(sessionId);
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

std::pair<unsigned int, unsigned int> ClubRepository::getLevelRange() const {
    SQLTransaction tx(d->db);
    return std::make_pair(
            getSettingValue<int>(skLevelMin).value_or(defaultLevelMin),
            getSettingValue<int>(skLevelMax).value_or(defaultLevelMax)
    );
}

bool ClubRepository::saveClubInfo(const QString &name, unsigned int levelMin, unsigned int levelMax) {
    if (levelMax < levelMin) {
        return false;
    }

    SQLTransaction tx(d->db);
    if (!saveSetting(skLevelMin, levelMin) || !saveSetting(skLevelMax, levelMax)) {
        tx.setError();
        return false;
    }

    if (!saveSetting(skClubName, name)) {
        tx.setError();
        return false;
    }

    return true;
}

size_t ClubRepository::importMembers(std::function<bool(Member &)> memberSupplier, QVector<Member> &failMembers) {
    SQLTransaction tx(d->db);

    size_t success = 0;
    Member member;
    while (memberSupplier(member)) {
        auto memberId = DbUtils::insert<MemberId>(
                d->db,
                QStringLiteral("insert into members (firstName, lastName, gender, level) values (?, ?, ?, ?)"),
                {member.firstName, member.lastName, enumToString(member.gender), member.level});

        if (!memberId) {
            failMembers.push_back(member);
        } else {
            success++;
        }
    }
    return success;
}


