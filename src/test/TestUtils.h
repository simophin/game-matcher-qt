//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_TESTUTILS_H
#define GAMEMATCHER_TESTUTILS_H

#include "models.h"

#include <QtTest/QtTest>

inline Member createMember(const char *firstName,
                           const char *lastName,
                           Member::Gender gender, int level) {
    Member m;
    m.firstName = QLatin1String(firstName);
    m.lastName = QLatin1String(lastName);
    m.gender = gender;
    m.level = level;
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

#endif //GAMEMATCHER_TESTUTILS_H
