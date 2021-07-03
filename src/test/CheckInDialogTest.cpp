//
// Created by Fanchao Liu on 26/08/20.
//

#include <catch2/catch.hpp>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

#include "CheckInDialog.h"
#include "ClubRepository.h"

#include <QSignalSpy>

TEST_CASE("CheckInDialog") {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, QStringLiteral(":memory:")));
    auto member = repo->createMember("First1", "Last1", BaseMember::Male, 1, "123", "email@email");
    REQUIRE(member.has_value());

    auto session = repo->createSession(500, "", "Announcement", 4, {
            {"Court1", 1},
    });
    REQUIRE(session.has_value());

    CheckInDialog dialog(member->id, session->session.id, repo.get());

    REQUIRE(dialog.findChild<QLabel *>("nameValueLabel")->text() == "First1 Last1");
    REQUIRE(dialog.findChild<QLabel *>("feeValueLabel")->text() == "5.00");
    REQUIRE(dialog.findChild<QLabel *>("announcement")->text() == "<u>Club announcement</u><br />Announcement");

    auto paidButton = dialog.findChild<QPushButton *>("paidButton");
    auto unpaidButton = dialog.findChild<QPushButton *>("unpaidButton");

    SECTION("Should check in member") {
        auto[buttonToClick, paidExpected] = GENERATE(table<QString, bool>(
                {
                        {"paidButton", true},
                        {"unpaidButton", false},
                }));

        QSignalSpy signalSpy(&dialog, &CheckInDialog::memberCheckedIn);

        dialog.findChild<QPushButton *>(buttonToClick)->click();
        auto members = repo->getMembers(CheckedIn{session->session.id});
        REQUIRE(members.size() == 1);
        REQUIRE(members[0].paid == paidExpected);
        REQUIRE(member == members[0]);

        REQUIRE(signalSpy.size() == 1);
        REQUIRE(signalSpy[0][0] == member->id);
    }
}