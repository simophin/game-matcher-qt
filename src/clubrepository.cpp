#include "clubrepository.h"

#include <QFile>
#include <QtDebug>
#include <QByteArray>
#include <QSqlResult>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


#include "dbutils.h"

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

    auto result = queryFirst<Setting>(db,
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
        emit lastGameInfoChanged();
        return true;
    }

    return false;
}


bool ClubRepository::isValid() const {
    return d->db.isOpen();
}

ClubInfo ClubRepository::clubInfo() const {
    auto result = query<Setting, QString>(d->db, QStringLiteral(
            "select * from settings where name in ('club_name', 'session_fee', 'club_creation_date')"));
    ClubInfo clubInfo;
    if (auto queryResult = std::get_if<QVector<Setting>>(&result)) {
        for (const auto &item : *queryResult) {
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

bool ClubRepository::setClubInfo(const ClubInfo &info) {
    SQLTransaction tx(d->db);

    auto saveSettings = [&](const QString &name, const QString &value) {
        auto result = query(d->db, QStringLiteral("insert or replace into settings (name, value) values (?, ?)"), name,
                            value);
        if (std::get_if<UpdateResult<int>>(&result)) {
            return true;
        }
        return false;
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

std::optional<SessionData> getSessionData(QSqlDatabase &db, SessionId sessionId) {
    auto session = queryFirst<Session>(db, QStringLiteral("select * from sessions where id = ?"), sessionId);

    if (!session) {
        return std::nullopt;
    }

    auto courtResult = query<Court>(db, QStringLiteral("select * from courts where sessionId = ?"), session->id);
    auto courts = std::get_if<QVector<Court>>(&courtResult);
    if (!courts) {
        return std::nullopt;
    }

    std::optional<SessionData> data;
    data.emplace().session = *session;
    data.value().courts = *courts;
    return data;
}

std::optional<SessionData> ClubRepository::getLastSession() const {
    QSqlQuery query(QStringLiteral("select id from sessions order by startTime desc limit 1"), d->db);
    if (query.next()) {
        return getSessionData(d->db, query.value(0).toInt());
    }

    return std::nullopt;
}

std::optional<SessionData>
ClubRepository::createSession(int fee, const QString &announcement, const QVector<CourtConfiguration> &courts) {
    SQLTransaction trans(d->db);
    auto result = query(d->db, QStringLiteral("insert into sessions (fee, announcement) values (?, ?)"), fee,
                        announcement);
    auto updateResult = std::get_if<UpdateResult<SessionId>>(&result);
    if (!updateResult || !updateResult->lastInsertedId) {
        trans.setError();
        return std::nullopt;
    }

    auto sessionId = *updateResult->lastInsertedId;

    for (const auto &court : courts) {
        result = query(d->db, QStringLiteral("insert into courts (sessionId, name, sortOrder) values (?, ?, ?)"),
                       sessionId, court.name, court.sortOrder);

        if (auto error = std::get_if<QSqlError>(&result); error) {
            qWarning() << "Error inserting court: " << *error;
            trans.setError();
            return std::nullopt;
        }
    }

    if (auto data = getSessionData(d->db, sessionId); data) {
        return data;
    }

    trans.setError();
    return std::nullopt;
}

QVector<GameAllocation> ClubRepository::getPastAllocations(SessionId id) const {
    auto queryResult = query<GameAllocation>(d->db,
                                             QStringLiteral("select * from game_allocations GA "
                                                            "inner join games G on GA.gameId = G.id "
                                                            "where G.sessionId = ?"),
                                             id);
    if (auto result = std::get_if<QVector<GameAllocation>>(&queryResult); result) {
        return *result;
    }

    return {};
}

std::optional<GameId> ClubRepository::createGame(SessionId sessionId, const QVector<GameAllocation> &allocations) {
    SQLTransaction tx(d->db);

    auto queryResult = query(d->db, QStringLiteral("insert into games (sessionId) values (?)"), sessionId);
    auto updateResult = std::get_if<UpdateResult<GameId>>(&queryResult);
    if (!updateResult || !updateResult->lastInsertedId) {
        tx.setError();
        return std::nullopt;
    }

    auto gameId = *updateResult->lastInsertedId;
    for (const auto &ga : allocations) {
        queryResult = query(d->db,
                            QStringLiteral("insert into game_allocations (gameId, courtId, playerId) values (?, ?, ?)"),
                            gameId, ga.courtId, ga.playerId);
        if (std::get_if<QSqlError>(&queryResult)) {
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

    auto result = query(d->db,
                        QStringLiteral("insert into members (firstName, lastName, gender, level) values (?, ?, ?, ?)"),
                        fistName, lastName, gender, level);

    auto updateResult = std::get_if<UpdateResult<MemberId>>(&result);
    if (!updateResult || !updateResult->lastInsertedId) {
        qWarning() << "Error inserting member";
        tx.setError();
        return std::nullopt;
    }

    return getMember(*updateResult->lastInsertedId);
}

QVector<MemberSearchResult> ClubRepository::findMember(const QString &needle) const {
    auto result = query<MemberSearchResult>(
            d->db,
            QStringLiteral("select M.*, highlight(member_names, 1, '[', ']') as matched from members M "
                           "inner join member_names on M.id = memberId "
                           "where member_names MATCH ? "
                           "order by rank"),
            needle);
    if (auto queryResult = std::get_if<QVector<MemberSearchResult>>(&result); queryResult) {
        return *queryResult;
    }

    return {};
}

std::optional<MemberInfo> ClubRepository::getMember(MemberId id) const {
    return queryFirst<MemberInfo>(d->db,
                                  QStringLiteral(
                                          "select M.*, M.initialBalance + (select COALESCE(sum(payment), 0) from players where memberId = M.id) as balance from members M where id = ?"),
                                  id);
}

std::optional<Player> ClubRepository::checkIn(MemberId memberId, SessionId sessionId, int payment) const {
    auto result = query(d->db,
                        QStringLiteral("insert into players (sessionId, memberId, payment) values (?, ?, ?)"),
                        sessionId, memberId, payment);
    auto updateResult = std::get_if<UpdateResult<PlayerId>>(&result);
    if (!updateResult || !updateResult->lastInsertedId) {
        return std::nullopt;
    }

    return getPlayer(*updateResult->lastInsertedId);
}

bool ClubRepository::checkOut(PlayerId id) {
    auto result = query(d->db,
                        QStringLiteral("update players set checkOutTime = current_timestamp where id = ?"),
                        id);
    if (auto updateResult = std::get_if<UpdateResult<PlayerId>>(&result)) {
        return updateResult->numRowsAffected > 0;
    }

    return false;
}

QVector<GameAllocation> ClubRepository::getLastGameAllocation(SessionId id) const {
    auto result = query<GameAllocation>(d->db,
                                        QStringLiteral("select GA.* from game_allocations GA "
                                                       "where GA.gameId in (select id from games order by startTime desc limit 1)"));
    if (auto queryResult = std::get_if<QVector<GameAllocation>>(&result)) {
        return *queryResult;
    }

    return {};
}

std::optional<Player> ClubRepository::getPlayer(PlayerId id) const {
    return queryFirst<Player>(d->db,
                              QStringLiteral("select * from players where id = ?"), id);
}

GameInfo ClubRepository::lastGameInfo() const {
    return GameInfo();
}


