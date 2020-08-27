//
// Created by Fanchao Liu on 27/08/20.
//

#include <catch2/catch.hpp>

#include "ClubRepository.h"
#include "EmptySessionPage.h"
#include "NewSessionDialog.h"
#include "ReportsDialog.h"
#include "MemberListDialog.h"
#include "MemberImportDialog.h"

#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>


TEST_CASE("EmptySessionPage") {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, ":memory:"));
    REQUIRE(repo->saveClubInfo("Club name", LevelRange{1, 4}));

    EmptySessionPage page(repo.get());
    CHECK(page.findChild<QLabel *>("clubNameLabel")->text() == "Welcome to Club name");
    auto resumeButton = page.findChild<QPushButton *>("resumeButton");
    CHECK(resumeButton->isEnabled() == false);

    SECTION("close button should ask to close") {
        QSignalSpy spy(&page, &EmptySessionPage::clubClosed);
        page.findChild<QPushButton *>("closeButton")->click();
        CHECK(spy.size() == 1);
    }

    SECTION("should resume last session") {
        QSignalSpy spy(&page, &EmptySessionPage::lastSessionResumed);

        REQUIRE(repo->createSession(500, "", "", 4, {{"Name1", 1}}));
        REQUIRE(resumeButton->isEnabled());
        resumeButton->click();
        CHECK(spy.size() == 1);
    }

    SECTION("should start a new session") {
        page.findChild<QPushButton *>("startButton")->click();

        auto dialog = page.findChild<NewSessionDialog *>();
        REQUIRE(dialog);

        REQUIRE(repo->createSession(500, "", "", 4, {{"Name1", 1}}));

        QSignalSpy spy(&page, &EmptySessionPage::newSessionCreated);
        dialog->sessionCreated();
        CHECK(spy.size() == 1);
    }

    SECTION("should show import dialog") {
        page.findChild<QPushButton *>("importButton")->click();
        CHECK(page.findChild<MemberImportDialog *>());
    }

    SECTION("should show report dialog") {
        page.findChild<QPushButton *>("reportButton")->click();
        CHECK(page.findChild<ReportsDialog *>());
    }

    SECTION("should members dialog") {
        page.findChild<QPushButton *>("membersButton")->click();
        CHECK(page.findChild<MemberListDialog *>());
    }
}