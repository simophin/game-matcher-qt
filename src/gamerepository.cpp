#include "gamerepository.h"

#include <QFile>
#include <QtSql>
#include <QtDebug>
#include <QByteArray>


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
    SQLTransaction(QSqlDatabase &db): db_(db) {
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
        if (auto v = query.record().value("value").toInt(&ok); ok) {
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

            auto sqls = QString::fromUtf8(schemaFile.readAll()).split(";");
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
