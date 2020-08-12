//
// Created by Fanchao Liu on 9/08/20.
//

#include "ClubRepository.h"

#include "TestUtils.h"

#include <catch2/catch.hpp>
#include <memory>
#include <QSignalSpy>

TEST_CASE("ClubRepository") {
    registerModels();

    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, ":memory:"));
    REQUIRE(repo);

    QSignalSpy clubChangeSpy(repo.get(), &ClubRepository::clubInfoChanged);
    QSignalSpy memberChangeSpy(repo.get(), &ClubRepository::memberChanged);
    QSignalSpy sessionChangeSpy(repo.get(), &ClubRepository::sessionChanged);

    SECTION("clubName") {
        auto name = GENERATE(QStringLiteral("Name 1"), QStringLiteral("Name 2"));

        REQUIRE(repo->getClubName().isEmpty());
        REQUIRE(repo->saveClubName(name));
        REQUIRE(repo->getClubName() == name);
    }

    SECTION("clubInfo") {
        auto[name, range, successExpected] = GENERATE(table<QString, LevelRange, bool>(
                {
                        {"Name1", {1, 2}, true},
                        {"Name1", {2, 1}, false},
                }));

        REQUIRE(repo->saveClubInfo(name, range) == successExpected);
        if (successExpected) {
            REQUIRE(repo->getClubName() == name);
            REQUIRE(repo->getLevelRange() == range);
        }
    }

    SECTION("settings") {
        auto[name, value] = GENERATE(table<SettingKey, QVariant>(
                {
                        {"key", "value1"},
                        {"key", 5},
                }));

        CHECK(!repo->getSetting(name));
        CHECK(repo->saveSetting(name, value));
        CHECK(repo->getSetting(name) == value.toString());
        CHECK(repo->removeSetting(name));
        CHECK(!repo->getSetting(name));
    }

    SECTION("member manipulation") {
        QVector<BaseMember> members(50);
        for (size_t i = 0, size = members.size(); i < size; i++) {
            auto &m = members[i];
            m.firstName = QStringLiteral("First%1").arg(i);
            m.lastName = QStringLiteral("Last%1").arg(i);
            m.gender = (i % 4 != 0) ? BaseMember::Male : BaseMember::Female;
            m.level = i;

            memberChangeSpy.clear();
            auto result = repo->createMember(m.firstName, m.lastName, m.gender, m.level);
            REQUIRE(result);
            m.id = result->id;
            m.registerDate = result->registerDate;
            REQUIRE(m.id >= 0);
            REQUIRE(m == result);
            REQUIRE(memberChangeSpy.size() == (result ? 1 : 0));
            REQUIRE(repo->getMember(m.id) == m);
        }
        memberChangeSpy.clear();

        SECTION("should not create duplicated member") {
            auto[first, last, successExpected] = GENERATE(table<QString, QString, bool>(
                    {
                            {"NoDuplicate", "Last",   true},
                            {"First1",      "Last1",  false},
                            {"first1",      "last1",  false},
                            {"first1",      "Last1",  false},
                            {"First1",      "last1",  false},
                            {"First1",      "last1 ", false},
                            {" First1",     "last1 ", false},
                    }));

            auto actual = repo->createMember(first, last, BaseMember::Male, 1);
            REQUIRE(actual.has_value() == successExpected);
            REQUIRE(memberChangeSpy.size() == (successExpected ? 1 : 0));
        }

        SECTION("name shouldn't be empty or blank") {
            auto[first, last, successExpected] = GENERATE(table<QString, QString, bool>(
                    {
                            {"OK ",    "Last ", true},
                            {"First1", "",      false},
                            {"First1", " ",     false},
                            {"",       "Last",  false},
                            {" ",      "Last",  false},
                    }));

            auto actual = repo->createMember(first, last, BaseMember::Female, 2);
            REQUIRE(actual.has_value() == successExpected);
            REQUIRE(memberChangeSpy.size() == (successExpected ? 1 : 0));
        }

        SECTION("should save") {
            auto[oldMember, newFirstName, newLastName, newGender, newLevel, successExpected] = GENERATE_COPY(table<
                    BaseMember,
                    std::optional<QString>,
                    std::optional<QString>,
                    std::optional<BaseMember::Gender>,
                    std::optional<int>,
                    bool
            >(
                    {
                            {members[0], "New first name", "New last name", BaseMember::Female, 1,            true},
                            {members[1], "First1",         "Last1",         std::nullopt,       std::nullopt, true},
                            {members[1], "First2",         "Last2",         std::nullopt,       std::nullopt, false},
                            {members[1], "First2 ",        "last2 ",        std::nullopt,       std::nullopt, false},
                    }));

            BaseMember toSave = oldMember;
            if (newFirstName) toSave.firstName = *newFirstName;
            if (newLastName) toSave.lastName = *newLastName;
            if (newGender) toSave.gender = *newGender;
            if (newLevel) toSave.level = *newLevel;

            REQUIRE(repo->saveMember(toSave) == successExpected);
            if (successExpected) {
                auto actual = repo->getMember(toSave.id);
                REQUIRE(actual.has_value());
                REQUIRE(toSave.id == actual->id);
                REQUIRE(toSave.firstName.trimmed() == actual->firstName.trimmed());
                REQUIRE(toSave.lastName.trimmed() == actual->lastName.trimmed());
                REQUIRE(toSave.gender == actual->gender);
                REQUIRE(toSave.level == actual->level);
                REQUIRE(toSave.registerDate == actual->registerDate);
            }

            REQUIRE(memberChangeSpy.size() == (successExpected ? 1 : 0));
        }

        SECTION("should find member by first & last name") {
            auto[firstName, lastName, expected] = GENERATE_COPY(table<QString, QString, std::optional<MemberId>>(
                    {
                            {"First0",  "Last0",  members[0].id},
                            {" First0", " Last0", members[0].id},
                            {"first0",  "last0",  members[0].id},
                            {"first0 ", " last0", members[0].id},
                            {"first0",  "",       std::nullopt},
                            {"",        "last0",  std::nullopt},
                            {"",        "",       std::nullopt},
                    }));

            auto actual = repo->findMemberBy(firstName, lastName);
            REQUIRE(actual == expected);
        }
    }

    SECTION("member import") {
        QVector<BaseMember> members(50);
        for (size_t i = 0, size = members.size(); i < size; i++) {
            auto &m = members[i];
            m.firstName = QStringLiteral("First%1").arg(i);
            m.lastName = QStringLiteral("Last%1").arg(i);
            m.gender = (i % 4 != 0) ? BaseMember::Male : BaseMember::Female;
            m.level = i;
        }

        members[1].firstName.clear();

        auto iter = members.begin();
        QVector<BaseMember> failed;
        auto numImported = repo->importMembers([&](auto &m) {
            if (iter == members.end()) return false;
            m = *iter;
            iter++;
            return true;
        }, &failed);

        REQUIRE(numImported == members.size() - 1);
        REQUIRE(!failed.isEmpty());
        REQUIRE(failed[0].firstName == members[1].firstName);
        REQUIRE(failed[0].lastName == members[1].lastName);
        REQUIRE(memberChangeSpy.size() == 1);

        auto allMembers = repo->getMembers(AllMembers{});
        std::sort(allMembers.begin(), allMembers.end());

        members.erase(members.begin() + 1);

        REQUIRE(members.size() == allMembers.size());
        for (size_t i = 0, size = members.size(); i < size; i++) {
            REQUIRE(members[i].firstName == allMembers[i].firstName);
            REQUIRE(members[i].lastName == allMembers[i].lastName);
            REQUIRE(members[i].gender == allMembers[i].gender);
            REQUIRE(members[i].level == allMembers[i].level);
        }
    }

    SECTION("session manipulation") {
        QVector<BaseMember> members;
        for (int i = 0; i < 50; i++) {
            auto m = repo->createMember(QStringLiteral("%1First").arg(i),
                                        QStringLiteral("%1Last").arg(i),
                                        i % 2 == 0 ? BaseMember::Male : BaseMember::Female,
                                        i % 4);
            REQUIRE(m);
            members.push_back(*m);
        }

        auto[fee, place, announcement, numPlayersPerCourt, courts, successExpected] = GENERATE(
                table<unsigned, QString, QString, unsigned, QVector<CourtConfiguration>, bool>(
                        {
                                {500, "Place 1", "Announcement 1", 4, {{"Court1", 1}, {"Court2", 2}}, true},
                                {0,   "Place 1", "Announcement 1", 4, {{"Court1", 1}, {"Court2", 2}}, true},
                                {0,   "",        "Announcement 1", 4, {{"Court1", 1}, {"Court2", 2}}, true},
                                {0,   "",        "",               4, {{"Court1", 1}, {"Court2", 2}}, true},
                                {500, "Place 1", "Announcement 1", 0, {{"Court1", 1}, {"Court2", 2}}, false},
                                {500, "Place 1", "Announcement 1", 4, {},                             false},
                        }));

        auto sessionData = repo->createSession(fee, place, announcement, numPlayersPerCourt, courts);
        REQUIRE(sessionData.has_value() == successExpected);
        if (!sessionData) return;

        auto sessionId = sessionData->session.id;

        REQUIRE(sessionChangeSpy.size() == 1);
        REQUIRE(sessionChangeSpy[0].first() == sessionId);
        sessionChangeSpy.clear();

        SECTION("getSession should work") {
            REQUIRE(repo->getSession(sessionId) == sessionData);
        }

        SECTION("getLastSession should work") {
            REQUIRE(repo->getLastSession() == sessionId);
        }

        SECTION("Checked in") {
            QVector<std::pair<BaseMember, bool>> checkedInMembers = {
                    {members[0], true},
                    {members[3], false},
                    {members[5], true},
                    {members[6], true},
                    {members[7], true},
            };

            QVector<std::pair<BaseMember, bool>> pausedMembers = {
                    checkedInMembers[1],
            };

            QVector<std::pair<BaseMember, bool>> checkedOutMembers = {
                    {members[8], false},
                    {members[9], true},
            };

            for (const auto &[m, paid] : checkedInMembers) {
                REQUIRE(repo->checkIn(sessionId, m.id, paid));
            }
            for (const auto &[m, paid]: checkedOutMembers) {
                REQUIRE(repo->checkIn(sessionId, m.id, paid));
            }

            for (const auto &m : pausedMembers) {
                REQUIRE(repo->setPaused(sessionId, m.first.id, true));
            }

            for (const auto &[m, paid]: checkedOutMembers) {
                REQUIRE(repo->checkOut(sessionId, m.id));
            }

            REQUIRE(sessionChangeSpy.size() ==
                    checkedInMembers.size() + checkedOutMembers.size() * 2 + pausedMembers.size());
            sessionChangeSpy.clear();
            memberChangeSpy.clear();

            SECTION("getMembers with session filter") {
                auto[filter, expected] = GENERATE_COPY(table<MemberSearchFilter, QVector<Member>>(
                        {
                                {
                                        AllSession{sessionId},
                                        {
                                                createMemberFrom(members[0], Member::CheckedIn, true),
                                                createMemberFrom(members[3], Member::CheckedInPaused, false),
                                                createMemberFrom(members[5], Member::CheckedIn, true),
                                                createMemberFrom(members[6], Member::CheckedIn, true),
                                                createMemberFrom(members[7], Member::CheckedIn, true),
                                                createMemberFrom(members[8], Member::CheckedOut, false),
                                                createMemberFrom(members[9], Member::CheckedOut, true),
                                        }
                                },
                                {
                                        CheckedIn{sessionId},
                                        {
                                                createMemberFrom(members[0], Member::CheckedIn, true),
                                                createMemberFrom(members[3], Member::CheckedInPaused, false),
                                                createMemberFrom(members[5], Member::CheckedIn, true),
                                                createMemberFrom(members[6], Member::CheckedIn, true),
                                                createMemberFrom(members[7], Member::CheckedIn, true),
                                        }
                                },
                                {
                                        CheckedIn{sessionId, true},
                                        {
                                                createMemberFrom(members[3], Member::CheckedInPaused, false),
                                        }
                                },
                                {
                                        CheckedIn{sessionId, false},
                                        {
                                                createMemberFrom(members[0], Member::CheckedIn, true),
                                                createMemberFrom(members[5], Member::CheckedIn, true),
                                                createMemberFrom(members[6], Member::CheckedIn, true),
                                                createMemberFrom(members[7], Member::CheckedIn, true),
                                        }
                                },
                        }));

                auto result = repo->getMembers(filter);
                std::sort(result.begin(), result.end());
                REQUIRE(result == expected);
            }

            SECTION("findMember with session filter") {
                auto[filter, needle, expected] = GENERATE_COPY(table<MemberSearchFilter, QString, QVector<Member>>(
                        {
                                {
                                        AllSession{sessionId},
                                        "0",
                                        {
                                                createMemberFrom(members[0], Member::CheckedIn, true),
                                        }
                                },
                                {
                                        CheckedIn{sessionId},
                                        "3",
                                        {
                                                createMemberFrom(members[3], Member::CheckedInPaused, false),
                                        }
                                },
                                {
                                        CheckedIn{sessionId, true},
                                        "5",
                                        {}
                                },
                                {
                                        CheckedIn{sessionId, false},
                                        "5",
                                        {
                                                createMemberFrom(members[5], Member::CheckedIn, true),
                                        }
                                },
                        }));

                auto result = repo->findMember(filter, needle);
                std::sort(result.begin(), result.end());
                REQUIRE(result == expected);
            }

            SECTION("Toggle paid") {
                auto expected = !checkedInMembers[0].second;
                REQUIRE(repo->setPaid(sessionId, checkedInMembers[0].first.id, expected));
                REQUIRE(sessionChangeSpy.size() == 1);
                REQUIRE(sessionChangeSpy.first().first() == sessionId);
                REQUIRE(memberChangeSpy.size() == 1);

                auto allCheckedIn = repo->getMembers(CheckedIn{sessionId});
                auto found = std::find_if(allCheckedIn.begin(), allCheckedIn.end(), [&](auto &m) {
                    return m.id == checkedInMembers[0].first.id;
                });
                REQUIRE(found != allCheckedIn.end());
                REQUIRE(found->paid == expected);
            }

            SECTION("Toggle pausing") {
                for (const auto &m : pausedMembers) {
                    REQUIRE(repo->setPaused(sessionId, m.first.id, false));
                }
                REQUIRE(sessionChangeSpy.size() == pausedMembers.size());
                REQUIRE(repo->getMembers(CheckedIn{sessionId, true}).isEmpty());
                auto actualMembers = repo->getMembers(CheckedIn{sessionId, false});
                std::sort(actualMembers.begin(), actualMembers.end());
                REQUIRE(actualMembers.size() == checkedInMembers.size());
                for (int i = 0, size = actualMembers.size(); i < size; i++) {
                    REQUIRE(checkedInMembers[i].first == actualMembers[i]);
                }
            }

            SECTION("Check in after check out") {
                auto member = createMemberFrom(checkedOutMembers[0].first, Member::CheckedIn,
                                               checkedOutMembers[0].second);
                REQUIRE(repo->checkIn(sessionId, member.id, member.paid.toBool()));
                auto allCheckedIn = repo->getMembers(CheckedIn{sessionId});
                auto found = std::find_if(allCheckedIn.begin(), allCheckedIn.end(), [&](const Member &m) {
                    return m.id == member.id;
                });
                REQUIRE(found != allCheckedIn.end());
                REQUIRE(*found == member);
            }

            SECTION("Game manipulation") {
                SECTION("should have no game") {
                    REQUIRE(repo->getLastGameInfo(sessionId) == std::nullopt);
                    REQUIRE(repo->getPastAllocations(sessionId).isEmpty());
                }

                auto &[allocations, duration, successExpected] = GENERATE_COPY(
                        table<QVector<GameAllocation>, qlonglong, bool>(
                                {
                                        {
                                                {},
                                                15,
                                                false
                                        },

                                        {
                                                {
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[0].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[1].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[2].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[3].first.id, 100),
                                                },
                                                15,
                                                true
                                        },
                                        {
                                                {
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[0].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[0].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[2].first.id, 100),
                                                        GameAllocation(0, sessionData->courts[0].id,
                                                                       checkedInMembers[3].first.id, 100),
                                                },
                                                15,
                                                false
                                        },
                                }));

                auto gameId = repo->createGame(sessionId, allocations, duration);
                REQUIRE(gameId.has_value() == successExpected);
                REQUIRE(sessionChangeSpy.size() == (successExpected ? 1 : 0));
                sessionChangeSpy.clear();
                if (!gameId) return;

                SECTION("getLastGameInfo should work") {
                    auto gameInfo = repo->getLastGameInfo(sessionId);
                    REQUIRE(gameInfo.has_value());

                    REQUIRE(gameInfo->durationSeconds == duration);
                    REQUIRE(gameInfo->id == *gameId);
                    REQUIRE(gameInfo->startDateTime().isValid());
                    REQUIRE(gameInfo->courts.size() == 1);
                    REQUIRE(gameInfo->courts.first().players.size() == allocations.size());
                    std::sort(gameInfo->courts.first().players.begin(), gameInfo->courts.first().players.end());
                    for (int i = 0, size = allocations.size(); i < size; i++) {
                        REQUIRE(gameInfo->courts.first().players[i].id == allocations[i].memberId);
                        REQUIRE(sessionData->courts[0].id == gameInfo->courts.first().courtId);
                        REQUIRE(allocations[i].quality == gameInfo->courts.first().courtQuality);
                    }

                    QVector<BaseMember> expectedWaiting;
                    for (const auto &item : checkedInMembers) {
                        auto inAllocation = std::find_if(allocations.begin(), allocations.end(), [&](auto &ga) {
                            return ga.memberId == item.first.id;
                        });
                        if (inAllocation == allocations.end()) {
                            expectedWaiting.push_back(item.first);
                        }
                    }

                    REQUIRE(expectedWaiting.size() == gameInfo->waiting.size());
                    std::sort(expectedWaiting.begin(), expectedWaiting.end());
                    std::sort(gameInfo->waiting.begin(), gameInfo->waiting.end());
                    for (int i = 0, size = expectedWaiting.size(); i < size; i++) {
                        REQUIRE(expectedWaiting[i] == gameInfo->waiting[i]);
                    }
                }

                SECTION("getPastAllocations should work") {
                    auto expectedAllocations = allocations;
                    for (auto &ga : expectedAllocations) {
                        ga.gameId = *gameId;
                    }

                    auto actual = repo->getPastAllocations(sessionId);
                    std::sort(actual.begin(), actual.end());
                    std::sort(expectedAllocations.begin(), expectedAllocations.end());
                    REQUIRE(actual == expectedAllocations);

                    auto newGameId = repo->createGame(sessionId, allocations, duration);
                    REQUIRE(newGameId);
                    for (auto &ga : allocations) {
                        expectedAllocations.push_back(ga);
                        expectedAllocations.last().gameId = *newGameId;
                    }

                    actual = repo->getPastAllocations(sessionId);
                    std::sort(actual.begin(), actual.end());
                    std::sort(expectedAllocations.begin(), expectedAllocations.end());
                    REQUIRE(actual == expectedAllocations);
                }

                SECTION("withdrawLastGame should work") {
                    REQUIRE(repo->withdrawLastGame(sessionId));
                    auto lastGame = repo->getLastGameInfo(sessionId);
                    if (lastGame) {
                        REQUIRE(lastGame->id != gameId);
                    }
                }
            }
        }
    }
}