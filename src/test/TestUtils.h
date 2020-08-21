//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_TESTUTILS_H
#define GAMEMATCHER_TESTUTILS_H

#include "models.h"
#include "TypeUtils.h"

#include <algorithm>
#include <type_traits>
#include <QtDebug>
#include <ostream>

inline Member createMember(const char *firstName,
                           const char *lastName,
                           Member::Gender gender = Member::Male, int level = 1) {
    Member m;
    m.firstName = QLatin1String(firstName);
    m.lastName = QLatin1String(lastName);
    m.gender = gender;
    m.level = level;
    return m;
}

template <typename T,
        std::enable_if_t<sqlx::HasMetaObject<T, const QMetaObject>::value, int> = 0,
        std::enable_if_t<std::is_base_of_v<QObject, T>, int> = 0>
inline std::ostream& operator << ( std::ostream& os, const T *value ) {
    const QMetaObject &obj = T::staticMetaObject;
    os << obj.className() << "{";
    for (int i = 0, size = obj.propertyCount(); i < size; i++) {
        auto p = obj.property(i);
        os << p.name() << " = " << (p.read(value).toString().toUtf8().data());
        if (i < size - 1) os << ", ";
    }
    os << "}";
    return os;
}

template <typename T,
        std::enable_if_t<sqlx::HasMetaObject<T, const QMetaObject>::value, int> = 0,
        std::enable_if_t<std::is_base_of_v<QObject, T>, int> = 0>
inline std::ostream& operator << ( std::ostream& os, const T &value ) {
    os << &value;
    return os;
}

template <typename T,
        std::enable_if_t<sqlx::HasMetaObject<T, const QMetaObject>::value, int> = 0,
        std::enable_if_t<!std::is_base_of_v<QObject, T>, int> = 0>
inline std::ostream& operator << ( std::ostream& os, const T *value ) {
    const QMetaObject &obj = T::staticMetaObject;
    os << obj.className() << "{";
    for (int i = 0, size = obj.propertyCount(); i < size; i++) {
        auto p = obj.property(i);
        os << p.name() << " = " << (p.readOnGadget(value).toString().toUtf8().data());
        if (i < size - 1) os << ", ";
    }
    os << "}";
    return os;
}

template <typename T,
        std::enable_if_t<sqlx::HasMetaObject<T, const QMetaObject>::value, int> = 0,
        std::enable_if_t<!std::is_base_of_v<QObject, T>, int> = 0>
inline std::ostream& operator << ( std::ostream& os, const T &value ) {
    os << &value;
    return os;
}

inline std::ostream& operator << ( std::ostream& os, const QString &value ) { return os << value.toUtf8().data(); }
inline std::ostream& operator << ( std::ostream& os, const QDateTime &value ) { return os << value.toString(); }

inline Member createMemberFrom(const BaseMember &base, Member::Status status, std::optional<bool> paid) {
    Member m;
    static_cast<BaseMember &>(m) = base;
    m.status = status;
    if (paid) {
        m.paid = *paid;
    }
    return m;
}

#endif //GAMEMATCHER_TESTUTILS_H
