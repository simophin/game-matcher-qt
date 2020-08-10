//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_TESTUTILS_H
#define GAMEMATCHER_TESTUTILS_H

#include "models.h"
#include "TypeUtils.h"

#include <QtTest/QtTest>
#include <algorithm>
#include <type_traits>
#include <QtDebug>

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

inline Member createMemberFrom(const BaseMember &base, Member::Status status, bool paid) {
    Member m;
    static_cast<BaseMember &>(m) = base;
    m.status = status;
    m.paid = paid;
    return m;
}

inline void verifyMember(const BaseMember &testSubject, const BaseMember &expected, const char *name) {
    QCOMPARE(testSubject.firstName, expected.firstName);
    QCOMPARE(testSubject.lastName, expected.lastName);
    QCOMPARE(testSubject.gender, expected.gender);
    QCOMPARE(testSubject.level, expected.level);
    QVERIFY2(testSubject.id > 0, name);
    QVERIFY2(testSubject.registerDate.isValid(), name);
}

template <typename MemberList>
inline void sortMembers(MemberList &members) {
    std::sort(members.begin(), members.end(), [](const BaseMember &lhs, const BaseMember &rhs) {
        return lhs.id < rhs.id;
    });
}

template <typename List1, typename List2>
inline void compareMembers(List1 actual, List2 expected, const char *testName) {
    QVERIFY2(actual.size() == expected.size(), testName);
    sortMembers(actual);
    sortMembers(expected);
    auto actualIter = actual.begin();
    auto expectedIter = expected.begin();
    while (actualIter != actual.end() && expectedIter != expected.end()) {
        verifyMember(*actualIter, *expectedIter, testName);
        actualIter++;
        expectedIter++;
    }
}

#endif //GAMEMATCHER_TESTUTILS_H
