#include "ClubRepository.h"
#include "ClubRepositoryInternal.h"

#include <QFile>
#include <QByteArray>
#include <QSqlResult>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


#include "DbUtils.h"

static struct {
    int schemaVersion;
    QString sqlFile;
} schemas[] = {
        {1, QStringLiteral(":/sql/db_v1.sql")},
        {2, QStringLiteral(":/sql/db_v2.sql")},
};

static const SettingKey skClubName = QStringLiteral("club_name");
static const SettingKey skClubCreationDate = QStringLiteral("club_creation_date");

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

ClubRepository::ClubRepository(QObject *parent) : QObject(parent), d(new Impl) {

}

ClubRepository::~ClubRepository() {
    delete d;
}

static std::optional<QSqlDatabase> openDatabase(const QString &dbPath, QString *errorString = nullptr) {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        if (errorString) {
            *errorString = db.lastError().text();
        }

        qWarning() << "Error opening: " << dbPath << ":" << db.lastError();
        return std::nullopt;
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
                return std::nullopt;
            }

            auto sqls = QString::fromUtf8(schemaFile.readAll()).split(QStringLiteral("---"));
            for (auto sql : sqls) {
                sql = sql.trimmed();
                if (sql.isEmpty()) continue;
                if (!q.exec(sql)) {
                    tx.setError();
                    if (errorString) {
                        *errorString = db.lastError().text();
                    }
                    qCritical() << "Error executing sql " << sql
                                << ":" << db.lastError();
                    return std::nullopt;
                }
            }
            qDebug() << "Migrated to schema version " << schema.schemaVersion;
        }
    }

    return db;
}

void ClubRepository::close() {
    d->db.close();
}

bool ClubRepository::open(const QString &path) {
    if (path == d->db.databaseName()) return d->db.isOpen();

    if (auto db = openDatabase(path)) {
        d->db = *db;
        emit clubInfoChanged();
        return true;
    }

    return false;
}


bool ClubRepository::isValid() const {
    return d->db.isOpen();
}

ClubInfo ClubRepository::getClubInfo() const {
    SQLTransaction tx(d->db);
    return {
            getSetting(skClubName),
            QDateTime::fromString(getSetting(skClubCreationDate))
    };
}

bool ClubRepository::saveClubInfo(const ClubInfo &info) {
    SQLTransaction tx(d->db);

    if (!saveSettings(skClubName, info.name) ||
        !saveSettings(skClubCreationDate, info.creationDate.toString())) {
        tx.setError();
        return false;
    }

    emit clubInfoChanged();
    return true;
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

QVector<GameAllocation> ClubRepository::getPastAllocations(SessionId id) const {
    return DbUtils::queryList<GameAllocation>(
            d->db,
            QStringLiteral("select GA.gameId, GA.courtId, P.memberId from game_allocations GA "
                           "inner join players P on P.id = GA.playerId "
                           "where GA.gameId in (select id from games where sessionId = ?) "),
            {id}).orDefault();
}

std::optional<GameId> ClubRepository::createGame(SessionId sessionId, const QVector<GameAllocation> &allocations) {
    SQLTransaction tx(d->db);

    auto gameId = DbUtils::insert<GameId>(d->db, QStringLiteral("insert into games (sessionId) values (?)"),
                                          {sessionId});
    if (!gameId) {
        tx.setError();
        return std::nullopt;
    }

    for (const auto &ga : allocations) {
        auto insertResult = DbUtils::update(
                d->db,
                QStringLiteral("insert into game_allocations (gameId, courtId, playerId) values (?, ?, "
                               "(select P.id from players P where P.memberId = ?) "
                               ")"),
                {*gameId, ga.courtId, ga.memberId});

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
                             const QString &lastName, const QString &gender, int level) {
    SQLTransaction tx(d->db);

    auto memberId = DbUtils::insert<MemberId>(
            d->db,
            QStringLiteral("insert into members (firstName, lastName, gender, level) values (?, ?, ?, ?)"),
            {fistName, lastName, gender, level});

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
        sql = QStringLiteral("select * from members M where 1");
    } else if (auto checkedIn = std::get_if<CheckedIn>(&filter)) {
        sql = QStringLiteral("select M.* from members M "
                             "inner join players P on P.memberId = M.id "
                             "where P.checkOutTime is null "
                             " and P.sessionId = ? ");
        args.push_back(checkedIn->sessionId);

        if (checkedIn->paused) {
            sql += QStringLiteral(" and P.paused = ?");
            args.push_back(*checkedIn->paused);
        }
    } else if (auto nonCheckedIn = std::get_if<NonCheckedIn>(&filter)) {
        sql = QStringLiteral("select M.* from members M "
                             "  where id not in (select memberId from players where sessionId = ?)");
        args.push_back(nonCheckedIn->sessionId);
    } else if (auto allSession = std::get_if<AllSession>(&filter)) {
        sql = QStringLiteral("select M.* from members M "
                             "inner join players P on P.memberId = M.id "
                             "where P.sessionId = ?");
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
    return DbUtils::queryList<Member>(d->db, sql, args).orDefault();
}

bool ClubRepository::checkIn(MemberId memberId, SessionId sessionId, bool paid) {
    auto rc = DbUtils::update(
            d->db,
            QStringLiteral("insert into players (sessionId, memberId, paid) values (?, ?, ?)"),
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
            QStringLiteral("select id, startTime from games "
                           "where sessionId = ? "
                           "order by startTime desc limit 1"),
            {sessionId});

    if (!gameResult) return std::nullopt;

    auto members = DbUtils::queryList<GameAllocationMember>(
            d->db,
            QStringLiteral("select M.*, C.id as courtId, C.name as courtName from game_allocations GA "
                           "inner join games G on G.id = GA.gameId "
                           "inner join players P on P.memberId = M.id and P.id = GA.playerId "
                           "inner join members M on M.id = P.memberId "
                           "inner join courts C on C.id = GA.courtId "
                           "where G.id = ? "
                           "order by C.sortOrder"),
            {gameResult->id, sessionId});

    if (!members) return std::nullopt;

    QHash<QString, Member *> firstNames;
    for (auto &member : *members) {
        if (auto found = firstNames.constFind(member.firstName); found != firstNames.constEnd()) {
            found.value()->displayName = found.value()->fullName();
            member.displayName = member.fullName();
        }
        firstNames[member.firstName] = &member;
    }
    firstNames.clear();

    for (auto &member : *members) {
        if (gameResult->courts.isEmpty() || gameResult->courts.last().courtId != member.courtId) {
            gameResult->courts.append(CourtPlayers{member.courtId, member.courtName});
        }

        gameResult->courts.last().players.append(member);
    }

    return *gameResult;
}

QString ClubRepository::getSetting(const SettingKey &key) const {
    return DbUtils::queryFirst<QString>(
            d->db, QStringLiteral("select value from settings where name = ?"), {key})
            .orDefault();
}

bool ClubRepository::saveSettings(const SettingKey &key, const QVariant &value) {
    return DbUtils::update(
            d->db,
            QStringLiteral("insert or replace into settings (name, value) values (?, ?)"),
            {key, value}).orDefault(0) > 0;
}

std::optional<Member> ClubRepository::getMember(MemberId id) const {
    auto[sql, args] = constructFindMembersSql(AllMembers{});
    sql += QStringLiteral(" and M.id = ?");
    args.push_back(id);

    return DbUtils::queryFirst<Member>(d->db, sql, args).toOptional();
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
            {m.firstName, m.lastName, m.gender, m.level, m.email, m.phone, m.id})
                   .orDefault(0) > 0;

}

bool ClubRepository::setPaused(SessionId sessionId, MemberId memberId, bool paused) {
    if (auto rc = DbUtils::update(d->db,
                           QStringLiteral("update players set paused = ? where sessionId = ? and memberId = ?"),
                           {paused, sessionId, memberId}).orDefault(0) > 0) {
        emit this->sessionChanged(sessionId);
        return true;
    }
    return false;
}


