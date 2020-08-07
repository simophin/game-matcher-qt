//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_CLUBREPOSITORYTEST_CPP
#define GAMEMATCHER_CLUBREPOSITORYTEST_CPP

#include "../ClubRepository.h"

#include <QtTest/QtTest>

#include "TestUtils.h"

class ClubRepositoryTest : public QObject {
Q_OBJECT
private slots :

    void init() {
        registerModels();
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

        QTest::newRow("Happy day") << "Test club name" << LevelRange{10, 12};
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
        QCOMPARE(value.toString(), repo->getSetting(key).value_or(QString()));
        QVERIFY(repo->removeSetting(key));
        QVERIFY(!repo->getSetting(key));
    }

    void testCreateMember() {
        struct {
            const char *name;
            BaseMember member;
            bool successExpected;
        } testData[] = {
                {
                        "Happy data",
                        createMember("First", "Last", Member::Male, 1),
                        true
                },
                {
                        "Duplicated not allowed",
                        createMember("First", "Last", Member::Male, 1),
                        false
                },
                {
                        "First name not allowed blank",
                        createMember("", "Last", Member::Male, 1),
                        false
                },
                {
                        "Last name not allowed blank",
                        createMember("First", "", Member::Male, 1),
                        false
                },
        };

        for (const auto &[name, member, successExpected] : testData) {
            auto actual = repo->createMember(member.firstName, member.lastName, member.gender, member.level);
            if (successExpected) {
                verifyMember(*actual, member, name);
            } else {
                QVERIFY2(!actual, name);
            }
        }
    }

    void testSaveMember() {
        struct {
            const char *name;
            Member member, updatedMember;
            bool successExpected;
        } testData[] = {
                {
                        "Happy day",
                        createMember("First1", "Last1", Member::Male, 1),
                        createMember("First1", "Last2", Member::Female, 2),
                        true,
                },
                {
                        "Name unchanged",
                        createMember("First2", "Last1", Member::Male, 1),
                        createMember("First2", "Last1", Member::Female, 2),
                        true
                },
                {
                        "Name change to existing name",
                        createMember("First3", "Last1", Member::Male, 1),
                        createMember("First2", "Last1", Member::Female, 2),
                        false
                },
        };

        for (auto &[name, member, updated, successExpected] : testData) {
            const auto created = repo->createMember(member.firstName, member.lastName, member.gender, member.level);
            QVERIFY2(created.has_value(), name);
            updated.id = created->id;
            QCOMPARE(repo->saveMember(updated), successExpected);

            const auto latest = repo->getMember(created->id);
            QVERIFY2(latest.has_value(), name);
            if (successExpected) {
                verifyMember(*latest, updated, name);
            } else {
                verifyMember(*latest, *created, name);
            }
        }


    }

    void testImportMembers() {
        struct {
            const char *name;
            QVector<BaseMember> inputMembers;
            size_t sizeExpected, failExpected;
        } testData[] = {
                {"Happy day",
                        {createMember("First", "Last", Member::Male, 2)},
                        1,
                        0
                },
        };

        for (auto &[name, inputMembers, sizeExpected, failExpected] : testData) {
            auto &list = inputMembers;
            auto iter = inputMembers.begin();
            QVector<BaseMember> failed;
            auto imported = repo->importMembers([&](BaseMember &m) {
                if (iter == list.end()) return false;
                m = *iter;
                iter++;
                return true;
            }, failed);
            QCOMPARE(imported, sizeExpected);
            QCOMPARE(failed.size(), failExpected);
        }
    }

    void cleanup() {
        delete repo;
    }

private:
    ClubRepository *repo = nullptr;
};


#endif //GAMEMATCHER_CLUBREPOSITORYTEST_CPP
