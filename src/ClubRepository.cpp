#include "ClubRepository.h"
#include "ClubRepositoryInternal.h"

#include <QFile>
#include <QtDebug>
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
};

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
    auto settings = DbUtils::queryList<Setting>(d->db, QStringLiteral(
            "select * from settings where name in ('club_name', 'session_fee', 'club_creation_date')"));
    ClubInfo clubInfo;
    if (settings) {
        for (const auto &item : *settings) {
            if (item.name == QLatin1String("club_name")) {
                clubInfo.name = item.value;
            } else if (item.name == QLatin1String("club_creation_date")) {
                clubInfo.creationDate = QDateTime::fromString(item.value);
            } else if (item.name == QLatin1String("session_fee")) {
                clubInfo.sessionFee = item.value.toInt();
            }
        }
        clubInfo.valid = !clubInfo.name.isEmpty();
    }
    return clubInfo;
}

bool ClubRepository::saveClubInfo(const ClubInfo &info) {
    SQLTransaction tx(d->db);

    auto saveSettings = [&](const QString &name, const QString &value) {
        return DbUtils::update(
                d->db,
                QStringLiteral("insert or replace into settings (name, value) values (?, ?)"),
                {name, value}) > 0;
    };

    if (!saveSettings(QStringLiteral("club_name"), info.name) ||
        !saveSettings(QStringLiteral("session_fee"), QString::number(info.sessionFee)) ||
        !saveSettings(QStringLiteral("club_creation_date"), info.creationDate.toString())) {
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
ClubRepository::createSession(int fee, const QString &announcement, int numPlayersPerCourt,
                              const QVector<CourtConfiguration> &courts) {
    SQLTransaction trans(d->db);
    auto sessionId = DbUtils::insert<SessionId>(
            d->db,
            QStringLiteral("insert into sessions (fee, announcement, numPlayersPerCourt) values (?, ?, ?)"),
            {fee, announcement, numPlayersPerCourt});
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
            QStringLiteral("select * from game_allocations where gameId in ( "
                           "select id from games where sessionId = ?"
                           ")"),
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
        auto insertResult = DbUtils::update(d->db,
                                            QStringLiteral(
                                                    "insert into game_allocations (gameId, courtId, playerId) values (?, ?, ?)"),
                                            {*gameId, ga.courtId, ga.playerId});
        if (!insertResult) {
            tx.setError();
            return std::nullopt;
        }
    }

    return gameId;
}

std::optional<MemberInfo>
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
    }

    return std::make_pair(sql, args);
}

QVector<Member> ClubRepository::findMember(MemberSearchFilter filter, const QString &needle) const {
    auto[sql, args] = constructFindMembersSql(filter);
    auto trimmed = needle;
    if (!trimmed.isEmpty()) {
        sql += QStringLiteral(" and (M.firstName like ? or M.lastName like ?)");
        auto realNeedle = QStringLiteral("%1%%").arg(trimmed);
        args.push_back(realNeedle);
        args.push_back(realNeedle);
    }

    return DbUtils::queryList<Member>(d->db, sql, args).orDefault();
}

QVector<Member> ClubRepository::getAllMembers(MemberSearchFilter filter) const {
    auto[sql, args] = constructFindMembersSql(filter);
    return DbUtils::queryList<Member>(d->db, sql, args).orDefault();
}

std::optional<MemberInfo> ClubRepository::getMember(MemberId id) const {
    return DbUtils::queryFirst<MemberInfo>(
            d->db,
            QStringLiteral(
                    "select M.*, M.initialBalance + (select COALESCE(sum(payment), 0) from players where memberId = M.id) as balance from members M where id = ?"),
            {id})
            .toOptional();
}

bool ClubRepository::checkIn(MemberId memberId, SessionId sessionId, int payment) const {
    return DbUtils::update(
            d->db,
            QStringLiteral("insert into players (sessionId, memberId, payment) values (?, ?, ?)"),
            {sessionId, memberId, payment}).orDefault(0) > 0;
}

bool ClubRepository::checkOut(SessionId sessionId, MemberId memberId) {
    return DbUtils::update(
            d->db,
            QStringLiteral("update players set checkOutTime = current_timestamp where sessionId = ? and memberId = ?"),
            {sessionId, memberId}).orDefault(0) > 0;
}

QVector<GameAllocation> ClubRepository::getLastGameAllocation(SessionId id) const {
    return DbUtils::queryList<GameAllocation>(
            d->db,
            QStringLiteral("select GA.* from game_allocations GA "
                           "where GA.gameId in (select id from games where sessionId = ? order by startTime desc limit 1)"),
            {id}).orDefault();
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
                           "order by courtId"),
            {gameResult->id});

    if (!members) return std::nullopt;

    for (const auto &member : *members) {
        if (gameResult->courts.isEmpty() || gameResult->courts.last().courtId != member.courtId) {
            gameResult->courts.append(CourtPlayers {member.courtId, member.courtName});
        }

        gameResult->courts.last().players.append(member);
    }

    return *gameResult;
}

QString ClubRepository::getSettings(const QString &key) const {
    return DbUtils::queryFirst<QString>(
            d->db, QStringLiteral("select value from settings where name = ?"), {key})
            .orDefault();
}

bool ClubRepository::setSettings(const QString &key, const QVariant &value) {
    return DbUtils::update(
            d->db,
            QStringLiteral("insert or replace into settings (name, value) values (?, ?)"),
            {key, value}).orDefault(0) > 0;
}

std::optional<SessionData> ClubRepository::getSession(SessionId sessionId) const {
    auto session = DbUtils::queryFirst<Session>(d->db, QStringLiteral("select * from sessions where id = ?"),
                                                {sessionId});

    if (!session) return std::nullopt;

    auto courts = DbUtils::queryList<Court>(d->db, QStringLiteral("select * from courts where sessionId = ?"),
                                            {sessionId});
    if (!courts) return std::nullopt;

    auto players = DbUtils::queryList<Member>(
            d->db,
            QStringLiteral("select M.* from members M inner join players P on M.id = P.memberId where P.sessionId = ?"),
            {sessionId});
    if (!players) return std::nullopt;

    std::optional<SessionData> data;
    data.emplace().session = *session;
    data.value().courts = *courts;
    data.value().checkedIn = *players;
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
    return DbUtils::update(d->db,
                           QStringLiteral("update players set paused = ? where sessionId = ? and memberId = ?"),
                           {paused, sessionId, memberId}).orDefault(0) > 0;
}

QVector<Player> ClubRepository::getAllPlayers(SessionId sessionId) const {
    return DbUtils::queryList<Player>(
            d->db,
            QStringLiteral("select * from players where sessionId = ?"), {sessionId})
            .orDefault();
}




