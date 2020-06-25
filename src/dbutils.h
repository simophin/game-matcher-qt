//
// Created by Fanchao Liu on 25/06/20.
//

#ifndef GAMEMATCHER_DBUTILS_H
#define GAMEMATCHER_DBUTILS_H

#include <QMetaObject>
#include <QMetaProperty>
#include <QSqlQuery>
#include <optional>
#include <QtDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QHash>
#include <QString>
#include <QVector>
#include <QVariant>
#include <variant>


inline void bindQuery(QSqlQuery &query, int) {}

template<typename T, typename ...Args>
inline void bindQuery(QSqlQuery &query, int startPos, T arg, Args...otherArgs) {
    query.bindValue(startPos, arg);
    bindQuery(query, startPos + 1, otherArgs...);
}

template<typename...Args>
bool prepareAndBindQuery(QSqlQuery &q, const QString &sql, Args...args) {
    if (!q.prepare(sql)) {
        return false;
    }

    bindQuery(q, 0, args...);
    return true;
}

template<typename Entity>
void writeRecordToEntity(const QSqlRecord &record, Entity &entity) {
    static auto propertyMaps = [] {
        QHash<QString, QMetaProperty> result;
        const QMetaObject *metaObject = &Entity::staticMetaObject;
        while (metaObject) {
            for (auto i = metaObject->propertyCount() - 1; i >= 0; i--) {
                auto prop = metaObject->property(i);
                result[QLatin1String(prop.name())] = prop;
            }
            metaObject = metaObject->superClass();
        }
        return result;
    }();

    for (int i = 0; i < record.count(); i++) {
        auto key = record.fieldName(i);
        auto prop = propertyMaps.constFind(key);
        if (prop == propertyMaps.constEnd()) {
            qWarning() << "Unable to find property " << key
                       << " in the entity: " << Entity::staticMetaObject.className();
            continue;
        }

        if (!prop->isWritable()) {
            qWarning() << "Unable to write to property: " << key
                       << " in the entity: " << Entity::staticMetaObject.className();
        }

        QVariant value;
        if (!record.isNull(i)) {
            value = record.value(i);
        }

        if (!prop->writeOnGadget(&entity, value)) {
            qWarning() << "Unable to write to property: " << key
                       << " in the entity: " << Entity::staticMetaObject.className()
                       << ", withValue = " << value;
        }
    }
}


struct VoidEntity {
Q_GADGET;
};


template<typename IdType = int>
struct UpdateResult {
    size_t numRowsAffected;
    std::optional<IdType> lastInsertedId;
};

template<typename Entity, typename IdType>
using QueryResult = std::variant<QSqlError, QVector<Entity>, UpdateResult<IdType>>;


template<typename Entity = VoidEntity, typename IdType = int, typename...Args>
QueryResult<Entity, IdType> query(QSqlDatabase &db, const QString &sql, Args...args) {
    QSqlQuery q(db);
    if (!prepareAndBindQuery(q, sql, args...) || !q.exec()) {
        qWarning() << "Error query: " << q.lastQuery() << ": " << q.lastError();
        return q.lastError();
    }

    if (q.isSelect()) {
        QVector<Entity> entities;
        entities.reserve(q.size());
        while (q.next()) {
            Entity entity;
            writeRecordToEntity(q.record(), entity);
            entities.append(entity);
        }

        return entities;
    }

    UpdateResult<IdType> result;

    if (q.lastInsertId().isValid()) {
        result.lastInsertedId = q.lastInsertId().value<IdType>();
    }
    result.numRowsAffected = q.numRowsAffected();

    return result;
}

template<typename Entity, typename...Args>
std::optional<Entity> queryFirst(QSqlDatabase &db, const QString &sql, Args...args) {
    auto result = query<Entity, Args...>(db, sql, args...);
    if (auto *entities = std::get_if<QVector<Entity>>(&result)) {
        if (entities->isEmpty()) {
            return std::nullopt;
        }
        return entities->at(0);
    }

    return std::nullopt;
}

#endif //GAMEMATCHER_DBUTILS_H
