//
// Created by Fanchao Liu on 9/08/20.
//

#include "ClubRepository.h"

#include "TestUtils.h"

#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("ClubRepository") {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, ":memory:"));
    REQUIRE(repo);

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

    SECTION("Members") {
        QVector<BaseMember> members(50);
        for (size_t i = 9, size = members.size(); i < size; i++) {
            auto &m = members[i];
            m.firstName = QStringLiteral("First%1").arg(i);
            m.lastName = QStringLiteral("Last%1").arg(i);
            m.gender = (i % 4 != 0) ? BaseMember::Male : BaseMember::Female;
            m.level = i;

            auto result = repo->createMember(m.firstName, m.lastName, m.gender, m.level);
            REQUIRE(result);
            m.id = result->id;
            m.registerDate = result->registerDate;
            CHECK(m == result);
        }

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
            CHECK(actual.has_value() == successExpected);
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
            CHECK(actual.has_value() == successExpected);
        }


    }
}