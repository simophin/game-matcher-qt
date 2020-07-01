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

#include <type_traits>

#include "TypeUtils.h"


template <typename T>
struct QueryResult {
    mutable std::variant<QSqlError, T> result;

    inline QueryResult(const QSqlError &e): result(e) {}
    inline QueryResult(const QSqlError *e): result(*e) {}
    inline QueryResult(const T &data): result(data) {}
    inline QueryResult() = default;

    inline T *success() const {
        return std::get_if<T>(&result);
    }

    inline QSqlError *error() const {
        return std::get_if<QSqlError>(&result);
    }

    inline T orDefault(T defaultValue = T()) {
        if (auto d = success()) {
            return *d;
        }
        return defaultValue;
    }

    inline std::optional<T> toOptional() {
        if (auto d = success()) {
            return *d;
        }
        return std::nullopt;
    }

    inline operator bool() const {
        return success() != nullptr;
    }

    inline T* operator->() {
        assert(success());
        return success();
    }

    inline T& operator*() {
        assert(success());
        return *success();
    }
};

class DbUtils {
public:

    template <typename Entity, std::enable_if_t<HasMetaObject<Entity>::value, int> = 0>
    static bool readFrom(Entity& entity, const QSqlRecord &record) {
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
        return true;
    }

    template <typename T, std::enable_if_t<std::is_constructible_v<QVariant, T>, int> = 0>
    static bool readFrom(T& out, const QSqlRecord &record) {
        if (auto v = record.value(0); v.isValid()) {
            out = v.value<T>();
            return true;
        }
        return false;
    }

    static QueryResult<QSqlQuery> buildQuery(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds);

    template <typename ResultType>
    static inline QueryResult<QVector<ResultType>> queryList(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds = {}) {
        auto query = buildQuery(db, sql, binds);
        if (!query) return query.error();
        if (!query->isSelect()) return {};
        QVector<ResultType> result;
        result.reserve(query->size());
        while (query->next()) {
            ResultType r;
            if (!readFrom(r, query->record())) {
                return QSqlError(QObject::tr("Unable to read from record"));
            }
            result.push_back(r);
        }
        return result;
    }

    template <typename ResultType>
    static inline QueryResult<ResultType> queryFirst(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds = {}) {
        auto result = queryList<ResultType>(db, sql, binds);
        if (!result) return result.error();
        if (result->isEmpty()) {
            return QSqlError(QObject::tr("Empty data set"));
        }
        return result->at(0);
    }

    template <typename IdType>
    static inline QueryResult<IdType> insert(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds = {}) {
        auto query = buildQuery(db, sql, binds);
        if (!query) return query.error();
        if (auto id = query->lastInsertId(); id.isValid()) {
            return id.value<IdType>();
        }
        return QSqlError(QObject::tr("Unable to retrieve lastInsertId"));
    }

    static inline QueryResult<int> update(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds = {}) {
        auto query = buildQuery(db, sql, binds);
        if (!query) return query.error();
        return query->numRowsAffected();
    }
};

#endif //GAMEMATCHER_DBUTILS_H
