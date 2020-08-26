//
// Created by Fanchao Liu on 26/08/20.
//

#include <catch2/catch.hpp>

#include <QSignalSpy>

#include "EditMemberDialog.h"
#include "ClubRepository.h"

#include <QComboBox>
#include <QLineEdit>

TEST_CASE("EditMemberDialog") {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, ":memory:"));
    REQUIRE(repo);

    REQUIRE(repo->saveClubInfo("club", LevelRange { 1, 4 }));

    EditMemberDialog dialog(repo.get());

    auto genderBox = dialog.findChild<QComboBox *>("genderComboBox");
    REQUIRE(genderBox);

    auto fullNameInput = dialog.findChild<QLineEdit *>("fullNameValue");
    REQUIRE(fullNameInput);

    auto levelBox = dialog.findChild<QComboBox *>("levelComboBox");
    REQUIRE(levelBox);

    auto emailInput = dialog.findChild<QLineEdit *>("emailLineEdit");
    REQUIRE(emailInput);

    auto phoneInput = dialog.findChild<QLineEdit *>("phoneLineEdit");
    REQUIRE(phoneInput);

    SECTION("new member") {
        for (int i = 0; i < 4; i++) {
            REQUIRE(levelBox->itemText(i).startsWith(QString::number(i + 1)));
        }
    }
}