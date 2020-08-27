//
// Created by Fanchao Liu on 27/08/20.
//
#include <catch2/catch.hpp>

#include <QPushButton>
#include <QSignalSpy>
#include <QSettings>

#include "MainWindow.h"
#include "ClubRepository.h"
#include "ClubPage.h"
#include "WelcomePage.h"
#include "NewClubDialog.h"

TEST_CASE("MainWindow") {
    QSettings settings;
    settings.clear();

    auto mainWindow = std::make_unique<MainWindow>();
    auto welcomePage = mainWindow->findChild<WelcomePage *>();
    REQUIRE(welcomePage);


    SECTION("Club opened") {
        welcomePage->findChild<QPushButton *>("createButton")->click();

        auto newClubPage = welcomePage->findChild<NewClubDialog *>();
        REQUIRE(newClubPage);

        newClubPage->setModal(false);
        newClubPage->clubCreated(":memory:");
        newClubPage->finished(0);

        REQUIRE(mainWindow->findChild<ClubPage *>());

        mainWindow.reset(new MainWindow());
        REQUIRE(mainWindow->findChild<ClubPage *>());
    }
}
