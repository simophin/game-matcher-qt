//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_CLUBREPOSITORYTEST_CPP
#define GAMEMATCHER_CLUBREPOSITORYTEST_CPP

#include "../ClubRepository.h"

#include <QtTest/QtTest>

class ClubRepositoryTest : public QObject {
    Q_OBJECT
private slots :
    void initTestCase() {
        repo = ClubRepository::open(this, QStringLiteral(":memory:"));
        QVERIFY(repo != nullptr);
    }

    void testClubName_data() {
        QTest::addColumn<QString>("clubName");

        QTest::newRow("Happy day") << "Test club name";
    }

    void testClubName() {
        QFETCH(QString, clubName);

        QVERIFY(repo->saveClubName(clubName));
        QCOMPARE(repo->getClubName(), clubName);
    }

    void testClubInfo_data() {
        QTest::addColumn<QString>("clubName");
        QTest::addColumn<LevelRange>("range");

        QTest::newRow("Happy day") << "Test club name" << LevelRange { 10, 12 };
    }

    void testClubInfo() {
        QFETCH(QString, clubName);
        QFETCH(LevelRange, range);

        QVERIFY(repo->saveClubInfo(clubName, range));
        QCOMPARE(repo->getLevelRange(), range);
        QCOMPARE(repo->getClubName(), clubName);
    }

    void testSettings_data() {
        QTest::addColumn<SettingKey>("key");
        QTest::addColumn<QVariant>("value");

        QTest::newRow("Strings") << "Key1" << QVariant::fromValue(QStringLiteral("Value1"));
        QTest::newRow("Integers") << "Key2" << QVariant::fromValue(5);
    }

    void testSettings() {
        QFETCH(SettingKey, key);
        QFETCH(QVariant, value);

        QVERIFY(repo->saveSetting(key, value));
        QCOMPARE(value.toString(), repo->getSetting(key));
    }

    void cleanupTestCase() {
        delete repo;
    }

private:
    ClubRepository *repo = nullptr;
};



#endif //GAMEMATCHER_CLUBREPOSITORYTEST_CPP
