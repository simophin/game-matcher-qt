//
// Created by Fanchao Liu on 26/08/20.
//

#include <catch2/catch.hpp>

#include <QSignalSpy>

#include "EditMemberDialog.h"
#include "ClubRepository.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>

TEST_CASE("EditMemberDialog") {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, ":memory:"));
    REQUIRE(repo);

    REQUIRE(repo->saveClubInfo("club", LevelRange { 1, 4 }));

    EditMemberDialog dialog(repo.get());

    QSignalSpy newMemberSignal(&dialog, &EditMemberDialog::newMemberCreated);
    QSignalSpy memberUpdatedSignal(&dialog, &EditMemberDialog::memberUpdated);

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

    for (int i = 0; i < 4; i++) {
        CHECK(levelBox->itemText(i).startsWith(QString::number(i + 1)));
        CHECK(levelBox->itemData(i) == i + 1);
    }

    CHECK(genderBox->itemData(0) == Member::Male);
    CHECK(genderBox->itemText(0) == "Male");
    CHECK(genderBox->itemData(1) == Member::Female);
    CHECK(genderBox->itemText(1) == "Female");

    CHECK(dialog.findChild<QMessageBox *>() == nullptr);

    SECTION("New member") {
        fullNameInput->setText("First Last");
        emailInput->setText("Email");
        phoneInput->setText("123");
        genderBox->setCurrentIndex(genderBox->findData(Member::Female));
        levelBox->setCurrentIndex(levelBox->findData(3));

        SECTION("with a unique name") {
            QString expectingEmail = "Email";
            QString expectingPhone = "123";

            SECTION("without email") {
                emailInput->clear();
                expectingEmail.clear();
            }

            SECTION("without phone") {
                phoneInput->clear();
                expectingPhone.clear();
            }

            dialog.accept();
            CHECK(newMemberSignal.size() == 1);
            CHECK(memberUpdatedSignal.isEmpty());

            auto savedMember = repo->getMember(newMemberSignal[0].first().value<MemberId>());
            REQUIRE(savedMember);

            CHECK(savedMember->firstName == "First");
            CHECK(savedMember->lastName == "Last");
            CHECK(savedMember->gender == Member::Female);
            CHECK(savedMember->email == expectingEmail);
            CHECK(savedMember->phone == expectingPhone);
            CHECK(savedMember->level == 3);
        }

        SECTION("validation") {
            SECTION("without name") {
                fullNameInput->clear();
            }

            SECTION("without full name") {
                fullNameInput->setText("First ");
            }

            SECTION("without a unique name") {
                REQUIRE(repo->createMember("First", "Last", Member::Female, 5, "123", "email"));
            }

            dialog.accept();
            CHECK(dialog.findChild<QMessageBox *>() != nullptr);
            CHECK(newMemberSignal.isEmpty());
            CHECK(memberUpdatedSignal.isEmpty());
        }
    }

    SECTION("Update member") {
        auto existingMember = repo->createMember("Update", "Last", Member::Male, 4, "012", "old_email");
        REQUIRE(existingMember);

        dialog.setMember(existingMember->id);
        CHECK(fullNameInput->text() == "Update Last");
        CHECK(levelBox->currentData() == 4);
        CHECK(genderBox->currentData() == Member::Male);
        CHECK(phoneInput->text() == "012");
        CHECK(emailInput->text() == "old_email");

        SECTION("Change stuff") {
            SECTION("Able to change name") {
                fullNameInput->setText("Updated Last");
                existingMember->firstName = "Updated";
            }

            SECTION("Able to change level") {
                levelBox->setCurrentIndex(levelBox->findData(3));
                existingMember->level = 3;
            }

            SECTION("Able to change email") {
                emailInput->setText("new email");
                existingMember->email = "new email";
            }

            SECTION("Able to change phone") {
                phoneInput->setText("new phone");
                existingMember->phone = "new phone";
            }

            dialog.accept();
            CHECK(dialog.findChild<QMessageBox *>() == nullptr);
            CHECK(newMemberSignal.isEmpty());
            REQUIRE(memberUpdatedSignal.size() == 1);
            CHECK(memberUpdatedSignal[0][0] == existingMember->id);
            CHECK(repo->getMember(existingMember->id) == existingMember);
        }
    }
}