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
Q_GADGET
public:
    typedef qlonglong IdType; // Wrong but to satisfy the compiler
};

struct SingleDataEntity {
Q_GADGET
public:
    typedef qlonglong IdType; // Wrong but to satisfy the compiler
    QVariant data;
    Q_PROPERTY(QVariant data MEMBER data);
};

template<typename EntityType = VoidEntity>
struct UpdateResult {
    size_t numRowsAffected;
    std::optional<typename EntityType::IdType> lastInsertedId;
};

template<typename Entity>
using QueryResult = std::variant<QSqlError, QVector<Entity>, UpdateResult<Entity>>;


template<typename Entity = VoidEntity, typename VariantList>
QueryResult<Entity> queryArgs(QSqlDatabase &db, const QString &sql, const VariantList &args) {
    QSqlQuery q(db);

    if (!q.prepare(sql)) {
        qWarning() << "Error preparing: " << q.lastQuery() << ": " << q.lastError();
        return q.lastError();
    }

    int i = 0;
    for (const auto &arg : args) {
        q.bindValue(i++, arg);
    }

    if (!q.exec()) {
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

    UpdateResult<Entity> result;

    if (q.lastInsertId().isValid()) {
        result.lastInsertedId = q.lastInsertId().value<typename Entity::IdType>();
    }
    result.numRowsAffected = q.numRowsAffected();

    return result;
}

template<typename Entity = VoidEntity, typename...Args>
QueryResult<Entity> query(QSqlDatabase &db, const QString &sql, Args...args) {
    return queryArgs<Entity>(db, sql, QVector<QVariant>{args...});
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
