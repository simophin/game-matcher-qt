#include "gamerepository.h"

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
    {1, QLatin1String(":/sql/db_v1.sql")},
};

class SQLTransaction {
    QSqlDatabase &db_;
    bool rollback_ = false;

public:
    explicit SQLTransaction(QSqlDatabase &db): db_(db) {
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

struct GameRepository::Impl {
    QSqlDatabase db;
};

GameRepository::GameRepository(QObject *parent): QObject(parent), d(new Impl)
{

}

GameRepository::~GameRepository()
{
    delete d;
}

bool GameRepository::open(const QString &dbPath, QString *errorString)
{
    auto db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"));
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        if (errorString) {
            *errorString = db.lastError().text();
        }

        qWarning() << "Error opening: " << dbPath << ":" << db.lastError();
        return false;
    }


    int currSchemaVersion = 0;

    SQLTransaction tx(db);

    QSqlQuery query(QLatin1String("select value from settings where name = 'schema_version'"), db);
    bool ok = false;
    if (query.isValid()) {
        if (auto v = query.value(QLatin1String("value")).toInt(&ok); ok) {
            currSchemaVersion = v;
        }
    }

    for (const auto & schema : schemas) {
        if (schema.schemaVersion > currSchemaVersion) {
            qDebug() << "Migrating to schema version " << schema.schemaVersion;
            QFile schemaFile(schema.sqlFile);
            if (!schemaFile.open(QIODevice::ReadOnly)) {
                tx.setError();
                qCritical() << "Unable to open file: " << schema.sqlFile;
                return false;
            }

            auto sqls = QString::fromUtf8(schemaFile.readAll()).split(QLatin1String(";"));
            for (auto sql : sqls) {
                sql = sql.trimmed();
                if (sql.isEmpty()) continue;
                if (!query.exec(sql)) {
                    tx.setError();
                    if (errorString) {
                        *errorString = db.lastError().text();
                    }
                    qCritical() << "Error executing sql from " << schema.sqlFile
                                << ":" << db.lastError().driverText();
                    return false;
                }
            }
            qDebug() << "Migrated to schema version " << schema.schemaVersion;
        }
    }

    d->db.close();
    d->db = db;
    return true;
}

std::optional<SessionData> getSessionData(QSqlDatabase &db, SessionId sessionId) {
    auto session = queryFirst<Session>(db, QLatin1String("select * from sessions where id = ?"), sessionId);

    if (!session) {
        return std::nullopt;
    }

    auto courtResult = query<Court>(db, QLatin1String("select * from courts where session_id = ?"), session->id);
    auto courts = std::get_if<QVector<Court>>(&courtResult);
    if (!courts) {
        return std::nullopt;
    }

    std::optional<SessionData> data;
    data.emplace().session = *session;
    data.value().courts = *courts;
    return data;
}

std::optional<SessionData> GameRepository::getLastSession() const {
    QSqlQuery query(QLatin1String("select id from sessions order by start_time desc limit 1"), d->db);
    if (query.next()) {
        return getSessionData(d->db, query.value(0).toInt());
    }

    return std::nullopt;
}

std::optional<SessionData> GameRepository::createSession(int fee, const QString &announcement, const QVector<CourtConfiguration> &courts) {
    SQLTransaction trans(d->db);
    auto result = query(d->db, QLatin1String("insert into sessions (fee, announcement) values (?, ?)"), fee, announcement);
    auto insertedId = std::get_if<QVariant>(&result);
    if (!insertedId || insertedId->isValid()) {
        return std::nullopt;
    }
    auto sessionId = insertedId->toInt();

    QSqlQuery courtQuery(d->db);
    if (!courtQuery.prepare(QLatin1String("insert into courts (session_id, name, sort_order) values (?, ?, ?)"))) {
        return std::nullopt;
    }

    for (const auto &court : courts) {
        bindQuery(courtQuery, sessionId, court.name, court.sortOrder);
        if (!courtQuery.exec()) {
            return std::nullopt;
        }
    }

    return getSessionData(d->db, sessionId);
}

QVector<GameAllocation> GameRepository::getPastAllocations(SessionId) const {
    return QVector<GameAllocation>();
}

GameId GameRepository::createGame(const QVector<GameAllocation> &) {
    return 0;
}
