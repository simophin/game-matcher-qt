//
// Created by Fanchao Liu on 7/08/20.
//

#ifndef GAMEMATCHER_CLUBREPOSITORYTEST_CPP
#define GAMEMATCHER_CLUBREPOSITORYTEST_CPP

#include "../ClubRepository.h"

#include <QtTest/QtTest>

#include "TestUtils.h"
#include "CollectionUtils.h"

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

        QVERIFY(!repo->getSetting(key));
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
                QVERIFY2(actual, name);
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
            auto iter = list.begin();
            QVector<BaseMember> failed;
            auto imported = repo->importMembers([&](BaseMember &m) {
                if (iter == list.end()) return false;
                m = *iter;
                iter++;
                return true;
            }, &failed);
            QCOMPARE(imported, sizeExpected);
            QCOMPARE(failed.size(), failExpected);
        }
    }

    void testFindMemberByNames() {
        QVector<BaseMember> members = {
                createMember("First1", "Last1"),
                createMember("First2", "Last2"),
                createMember("First3", "Last3"),
        };

        for (auto &item : members) {
            item = *repo->createMember(item.firstName, item.lastName, item.gender, item.level);
        }

        struct {
            const char *testName;
            std::pair<const char *, const char *> names;
            std::optional<MemberId> expected;
        } testData[] = {
                {"Exact match",
                        std::make_pair("First1", "Last1"),
                        members[0].id},

                {"Case insensitive first name",
                        std::make_pair("first1", "Last1"),
                        members[0].id},

                {"Case insensitive last name",
                        std::make_pair("First1", "last1"),
                        members[0].id},

                {"Case insensitive names",
                        std::make_pair("first1", "last1"),
                        members[0].id},

                {"Found nothing",
                        std::make_pair("First4", "Last4"),
                        std::nullopt
                }
        };

        for (auto &[testName, names, expected] : testData) {
            auto actual = repo->findMemberBy(
                    QLatin1String(names.first), QLatin1String(names.second));
            QCOMPARE(actual, expected);
        }
    }

    void testCreateSession() {
        struct {
            const char *testName;
            unsigned fee;
            QString place;
            QString announcement;
            unsigned numPlayersPerCourt;
            QVector<CourtConfiguration> courtConfigurations;
            bool expectedSuccess;
        } testData[] = {
                {
                        "Happy day",
                        5,
                        QStringLiteral("Place 1"),
                        QStringLiteral("Annoucement 1"),
                        4,
                        {
                                {QStringLiteral("1"), 1},
                                {QStringLiteral("2"), 2},
                        },
                        true
                },
                {
                        "No fee",
                        0,
                        QStringLiteral("Place 1"),
                        QStringLiteral("Annoucement 1"),
                        4,
                        {
                                {QStringLiteral("1"), 1},
                                {QStringLiteral("2"), 2},
                        },
                        true
                },
                {
                        "No announcement",
                        5,
                        QStringLiteral("Place 1"),
                        QString(),
                        4,
                        {
                                {QStringLiteral("1"), 1},
                                {QStringLiteral("2"), 2},
                        },
                        true
                },
                {
                        "No court",
                        5,
                        QStringLiteral("Place 1"),
                        QString(),
                        4,
                        {},
                        false
                },
                {
                        "zero numPlayersPerCourt",
                        5,
                        QStringLiteral("Place 1"),
                        QStringLiteral("Announcement"),
                        0,
                        {
                                {QStringLiteral("1"), 1},
                                {QStringLiteral("2"), 2},
                        },
                        false
                }
        };

        for (const auto &d : testData) {
            auto session = repo->createSession(d.fee, d.place, d.announcement, d.numPlayersPerCourt,
                                               d.courtConfigurations);
            if (d.expectedSuccess) {
                QVERIFY2(session.has_value(), d.testName);
                QCOMPARE(*repo->getLastSession(), session->session.id);
                QCOMPARE(repo->getSession(session->session.id), session);
                QCOMPARE(session->session.place, d.place);
                QCOMPARE(session->session.numPlayersPerCourt, d.numPlayersPerCourt);
                QCOMPARE(session->session.announcement, d.announcement);
                QCOMPARE(session->session.fee, d.fee);
                QVERIFY2(session->session.startTime.isValid(), d.testName);
                QVERIFY2(session->session.id > 0, d.testName);
            } else {
                QVERIFY2(!session.has_value(), d.testName);
            }
        }
    }

    void testMemberGameOperation() {
        QVector<BaseMember> members = {
                createMember("First", "Last1", Member::Male, 1),
                createMember("First", "Last2", Member::Female, 2),
                createMember("First", "Last3", Member::Male, 3),
        };

        for (auto &item : members) {
            item = *repo->createMember(item.firstName, item.lastName, item.gender, item.level);
        }

        auto membersById = associateBy<QHash<MemberId, BaseMember>>(members, [](auto &m) {
            return m.id;
        });

        compareMembers(repo->getMembers(AllMembers{}), members, "testMemberGameOperation");

        typedef bool Paid;

        struct {
            const char *testName;
            std::map<MemberId, Paid> checkInMembers;
            std::map<MemberId, Paid> checkOutMembers;
            QSet<MemberId> paused;
        } testData[] = {
                {
                        "No one checkout nor pause",
                        {
                                std::make_pair(members[0].id, false),
                                std::make_pair(members[1].id, false),
                        },
                        {},
                        {}
                },
                {
                        "No on check in",
                        {},
                        {},
                        {}
                },
                {
                        "Somebody pauses, nobody checks out",
                        {
                                std::make_pair(members[0].id, true),
                                std::make_pair(members[1].id, true),
                        },
                        {},
                        {members[0].id}
                },
                {
                        "Somebody checked out, nobody pauses",
                        {
                                std::make_pair(members[0].id, true),
                        },
                        {
                                std::make_pair(members[1].id, true),
                        },
                        {}
                },
                {
                        "Somebody checked out, somebody pauses",
                        {
                                std::make_pair(members[0].id, true),
                                std::make_pair(members[2].id, true),
                        },
                        {
                                std::make_pair(members[1].id, false),
                        },
                        {
                         members[2].id
                        }
                },
        };

        for (const auto &d : testData) {
            auto session = repo->createSession(0, QStringLiteral("Place1"), QString(), 4, {{QStringLiteral("1"), 1}});
            QVERIFY2(session.has_value(), d.testName);

            for (auto[memberId, paid] : d.checkInMembers) {
                QVERIFY2(repo->checkIn(session->session.id, memberId, paid), d.testName);
            }

            for (auto[memberId, paid] : d.checkOutMembers) {
                QVERIFY2(repo->checkIn(session->session.id, memberId, paid), d.testName);
            }

            for (const auto &item : d.paused) {
                QVERIFY2(repo->setPaused(session->session.id, item, true), d.testName);
            }

            for (const auto &[memberId, paid] : d.checkOutMembers) {
                QVERIFY2(repo->checkOut(session->session.id, memberId), d.testName);
            }

            // Test non checked in filter
            {
                auto nonCheckedIn = associateBy<QHash<MemberId, BaseMember>>(
                        members,
                        [](auto &m) {
                            return m.id;
                        });

                for (auto[memberId, paid] : d.checkInMembers) {
                    nonCheckedIn.remove(memberId);
                }

                auto actual = repo->getMembers(NonCheckedIn{session->session.id});
                compareMembers(actual, nonCheckedIn.values(), d.testName);
                for (const auto &m : actual) {
                    QCOMPARE(Member::NotCheckedIn, m.status);
                }
            }

            // Test check in filter
            {
                auto actualCheckedIn = repo->getMembers(CheckedIn{session->session.id});
                QCOMPARE(actualCheckedIn.size(), d.checkInMembers.size());
                for (const auto &m : actualCheckedIn) {
                    auto found = d.checkInMembers.find(m.id);
                    QVERIFY2(found != d.checkInMembers.end(), d.testName);
                    verifyMember(m, membersById[m.id], d.testName);
                    QCOMPARE(found->second, m.paid.toBool());
                    if (d.paused.contains(m.id)) {
                        QCOMPARE(Member::CheckedInPaused, m.status);
                    } else {
                        QCOMPARE(Member::CheckedIn, m.status);
                    }
                }
            }

            // Test check in non pause filter
            {
                QVector<BaseMember> expected;
                for (const auto &[memberId, paid] : d.checkInMembers) {
                    if (!d.paused.contains(memberId)) {
                        expected.push_back(membersById[memberId]);
                    }
                }

                auto actual = repo->getMembers(CheckedIn{session->session.id, false});
                compareMembers(actual, expected, d.testName);
                for (const auto &m : actual) {
                    QCOMPARE(Member::CheckedIn, m.status);
                }
            }

            // Test check in pause filter
            {
                QVector<BaseMember> expected;
                for (const auto &[memberId, paid] : d.checkInMembers) {
                    if (d.paused.contains(memberId)) {
                        expected.push_back(membersById[memberId]);
                    }
                }

                auto actual = repo->getMembers(CheckedIn{session->session.id, true});
                compareMembers(actual, expected, d.testName);
                for (const auto &m : actual) {
                    QCOMPARE(Member::CheckedInPaused, m.status);
                }
            }

            // Test AllSession filter
            {
                QVector<BaseMember> expected;
                for (const auto &[memberId, paid] : d.checkInMembers) {
                    expected.push_back(membersById[memberId]);
                }
                for (const auto &[memberId, paid] : d.checkOutMembers) {
                    expected.push_back(membersById[memberId]);
                }

                auto actual = repo->getMembers(AllSession{session->session.id});
                compareMembers(actual, expected, d.testName);
                for (const auto &m : actual) {
                    if (d.checkOutMembers.find(m.id) != d.checkOutMembers.end()) {
                        QCOMPARE(Member::CheckedOut, m.status);
                    } else if (d.paused.contains(m.id)) {
                        QCOMPARE(Member::CheckedInPaused, m.status);
                    } else {
                        QCOMPARE(Member::CheckedIn, m.status);
                    }
                }
            }
        }
    }

    void testGameAllocation() {
        QVector<BaseMember> members = {
                createMember("First1", "Last1"),
                createMember("First2", "Last2"),
                createMember("First3", "Last3"),
                createMember("First4", "Last4"),
        };

        for (auto &item : members) {
            item = *repo->createMember(item.firstName, item.lastName, item.gender, item.level);
        }

        auto session = repo->createSession(
                0, QStringLiteral("Place"), QString(), 2, {
                        {QStringLiteral("1"), 1},
                        {QStringLiteral("2"), 2}
                });
        QVERIFY(session);

        for (const auto &item : members) {
            QVERIFY(repo->checkIn(session->session.id, item.id, true));
        }

        struct {
            const char *testName;
            QVector<GameAllocation> allocated;
            qlonglong durationSeconds;
            bool successExpected;
        } testData[] = {
                {
                        "No allocation",
                        {},
                        30,
                        false
                },
                {
                        "Full courts",
                        {
                                GameAllocation(0, session->courts[0].id, members[0].id, 100),
                                GameAllocation(0, session->courts[0].id, members[1].id, 100),
                                GameAllocation(0, session->courts[1].id, members[2].id, 80),
                                GameAllocation(0, session->courts[1].id, members[3].id, 80),
                        },
                        30,
                        true
                },
                {
                        "Partial courts",
                        {
                                GameAllocation(0, session->courts[0].id, members[3].id, 100),
                                GameAllocation(0, session->courts[0].id, members[1].id, 100),
                                GameAllocation(0, session->courts[1].id, members[2].id, 80),
                                GameAllocation(0, session->courts[1].id, members[0].id, 80),
                        },
                        20,
                        true
                },
                {
                        "Incorrect member in court",
                        {
                                GameAllocation(0, session->courts[0].id, members[3].id, 100),
                                GameAllocation(0, session->courts[0].id, members[3].id, 100),
                                GameAllocation(0, session->courts[1].id, members[2].id, 80),
                                GameAllocation(0, session->courts[1].id, members[0].id, 80),
                        },
                        20,
                        false
                }
        };

        QVector<GameAllocation> expected;
        for (auto &d : testData) {
            auto gameId = repo->createGame(session->session.id, d.allocated, d.durationSeconds);
            if (!d.successExpected) {
                QVERIFY(!gameId.has_value());
                continue;
            }

            for (auto &ga : d.allocated) {
                ga.gameId = *gameId;
            }

            QVERIFY(gameId.has_value());

            for (const auto &ga : d.allocated) {
                expected.push_back(GameAllocation(*gameId, ga.courtId, ga.memberId, ga.quality));
            }

            auto gameInfo = repo->getLastGameInfo(session->session.id);
            QVERIFY(gameInfo);
            QCOMPARE(*gameId, gameInfo->id);
            QVERIFY(gameInfo->startTime > 0);
            QCOMPARE(gameInfo->durationSeconds, d.durationSeconds);

            {
                QVector<GameAllocation> actual, expectedAllocations;
                for (const auto &court : gameInfo->courts) {
                    for (const auto &player : court.players) {
                        actual.push_back(GameAllocation(gameInfo->id, court.courtId, player.id, court.courtQuality));
                    }
                }
                expectedAllocations = d.allocated;
                std::sort(actual.begin(), actual.end());
                std::sort(expectedAllocations.begin(), expectedAllocations.end());
                QVERIFY2(actual == expectedAllocations, d.testName);
            }

            QCOMPARE(repo->getPastAllocations(session->session.id), expected);
        }
    }

    void testSetPaid() {
        auto member = repo->createMember(
                QStringLiteral("First1"),
                QStringLiteral("Last1"), Member::Male, 10);
        QVERIFY(member);

        auto session = repo->createSession(0, QStringLiteral("Place"), QString(), 2, {
                {QStringLiteral("1"), 1},
                {QStringLiteral("2"), 2}
        });
        QVERIFY(session);

        QVERIFY(repo->checkIn(session->session.id, member->id, false));
        auto members = repo->getMembers(AllSession{session->session.id});
        QVERIFY(!members.isEmpty());
        QCOMPARE(false, members.first().paid);

        QVERIFY(repo->setPaid(session->session.id, member->id, true));
        members = repo->getMembers(AllSession{session->session.id});
        QVERIFY(!members.isEmpty());
        QCOMPARE(true, members.first().paid);
    }

    void testFindMember() {
        QVector<BaseMember> members = {
                createMember("Able", "Last1"),
                createMember("Bee", "Last2"),
                createMember("Clinton", "Last3"),
                createMember("Dog", "Last4"),
                createMember("Ella", "Last4"),
                createMember("Ellie", "Last5"),
        };

        for (auto &item : members) {
            item = *repo->createMember(item.firstName, item.lastName, item.gender, item.level);
        }

        struct {
            const char *testName;
            MemberSearchFilter filter;
            QString needle;
            QVector<BaseMember> expected;
        } testData[] = {
                {
                        "Partial match",
                        AllMembers{},
                        QStringLiteral("Ab"),
                        {members[0]},
                },
                {
                        "Partial case-insensitive match",
                        AllMembers{},
                        QStringLiteral("ab"),
                        {members[0]},
                },
                {
                        "Full case-insensitive match",
                        AllMembers{},
                        QStringLiteral("Able"),
                        {members[0]},
                },
                {
                        "Multiple result",
                        AllMembers{},
                        QStringLiteral("Ell"),
                        {members[4], members[5],}
                },
                {
                        "Non result",
                        CheckedIn{1},
                        QStringLiteral("Able"),
                        {},
                }
        };

        for (const auto &d : testData) {
            auto actual = repo->findMember(d.filter, d.needle);
            compareMembers(actual, d.expected, d.testName);
        }
    }



    void cleanup() {
        delete repo;
    }

private:
    ClubRepository *repo = nullptr;
};

QTEST_MAIN(ClubRepositoryTest)

#include "ClubRepositoryTest.moc"


#endif //GAMEMATCHER_CLUBREPOSITORYTEST_CPP
