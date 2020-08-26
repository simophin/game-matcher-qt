//
// Created by Fanchao Liu on 26/08/20.
//

#include "CourtDisplay.h"
#include "ClubRepositoryModels.h"

#include <catch2/catch.hpp>

#include <QLabel>

static auto create(const QString &firstName,
                   const QString &lastName,
                   Member::Status status,
                   bool paid) {
    static MemberId sId = 1;
    Member m;
    m.id = ++sId;
    m.firstName = firstName;
    m.lastName = lastName;
    m.level = 1 + (m.id % 4);
    m.paid = paid;
    m.status = status;
    m.phone = QStringLiteral("Phone %1").arg(m.id);
    m.email = QStringLiteral("Email %1").arg(m.id);
    m.gender = Member::Male;
    return m;
}

TEST_CASE("CourtDisplay") {
    CourtDisplay courtDisplay;

    courtDisplay.setCourt(CourtPlayers{
            1, "Name 1", 100,
            {
                create("First1", "Last1", Member::CheckedIn, true),
                create("First2", "Last2", Member::CheckedInPaused, true),
                create("First3", "Last3", Member::CheckedOut, true),
                create("First4", "Last4", Member::CheckedIn, false),
            }
    });

    auto labels = courtDisplay.findChildren<QLabel *>();
    REQUIRE(labels.size() == 5);
    REQUIRE(labels[0]->text() == "Court Name 1");

    REQUIRE(labels[1]->text() == "First1 Last1");
    REQUIRE(!labels[1]->font().strikeOut());
    REQUIRE(!labels[1]->font().underline());

    REQUIRE(labels[2]->text() == "First2 Last2");
    REQUIRE(labels[2]->font().strikeOut());
    REQUIRE(!labels[2]->font().underline());

    REQUIRE(labels[3]->text() == "First3 Last3");
    REQUIRE(labels[3]->font().strikeOut());
    REQUIRE(!labels[3]->font().underline());

    REQUIRE(labels[4]->text() == "First4 Last4");
    REQUIRE(!labels[4]->font().strikeOut());
    REQUIRE(labels[4]->font().underline());


    courtDisplay.setCourt(CourtPlayers{
            1, "Name 2", 100,
            {
                    create("First2_1", "Last1", Member::CheckedIn, true),
                    create("First2_2", "Last2", Member::CheckedInPaused, true),
                    create("First2_3", "Last3", Member::CheckedOut, true),
                    create("First2_4", "Last4", Member::CheckedIn, false),
            }
    });

    labels = courtDisplay.findChildren<QLabel *>();
    REQUIRE(labels.size() == 5);
    REQUIRE(labels[0]->text() == "Court Name 2");

    REQUIRE(labels[1]->text() == "First2_1 Last1");
    REQUIRE(!labels[1]->font().strikeOut());
    REQUIRE(!labels[1]->font().underline());

    REQUIRE(labels[2]->text() == "First2_2 Last2");
    REQUIRE(labels[2]->font().strikeOut());
    REQUIRE(!labels[2]->font().underline());

    REQUIRE(labels[3]->text() == "First2_3 Last3");
    REQUIRE(labels[3]->font().strikeOut());
    REQUIRE(!labels[3]->font().underline());

    REQUIRE(labels[4]->text() == "First2_4 Last4");
    REQUIRE(!labels[4]->font().strikeOut());
    REQUIRE(labels[4]->font().underline());
}