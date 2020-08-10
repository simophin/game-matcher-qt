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
}