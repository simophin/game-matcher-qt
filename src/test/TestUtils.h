//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_TESTUTILS_H
#define GAMEMATCHER_TESTUTILS_H

#include "models.h"

#include <QtTest/QtTest>
#include <algorithm>

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

template <typename List1, typename List2>
inline void compareGameAllocations(List1 actual, List2 expected, const char *testName) {
    QVERIFY2(actual.size == expected.size, testName);

    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), actual.end());

    auto actualIter = actual.begin();
    auto expectedIter = expected.begin();
    while (actualIter != actual.end() && expectedIter != expected.end()) {
        QCOMPARE(*actualIter, *expectedIter);
        actualIter++;
        expectedIter++;
    }
}

#endif //GAMEMATCHER_TESTUTILS_H
