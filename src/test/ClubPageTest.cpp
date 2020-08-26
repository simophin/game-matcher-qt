//
// Created by Fanchao Liu on 26/08/20.
//

#include "ClubPage.h"

#include <catch2/catch.hpp>
#include <QSignalSpy>
#include <QLayout>

#include "ClubRepository.h"
#include "EmptySessionPage.h"
#include "SessionPage.h"

TEST_CASE("ClubPage") {
    std::unique_ptr<ClubPage> page(ClubPage::create(":memory:", nullptr));
    auto repo = page->clubRepository();

    auto emptyPage = page->findChild<EmptySessionPage *>();
    REQUIRE(emptyPage);

    SECTION("should fire club closed signal") {
        QSignalSpy spy(page.get(), &ClubPage::clubClosed);
        emptyPage->clubClosed();

        REQUIRE(spy.size() == 1);
    }

    SECTION("session related") {
        auto member = repo->createMember("First", "Last", Member::Male, 1, "123", "234");
        REQUIRE(member);

        auto session = repo->createSession(500, "", "", 4, { { "Court1", 1 } });
        REQUIRE(session);

        SECTION("should display newly created session page") {
            emptyPage->newSessionCreated();
            auto sessionPage = page->findChild<SessionPage *>();
            REQUIRE(sessionPage != nullptr);

            SECTION("should fire full screen request") {
                QSignalSpy spy(page.get(), &ClubPage::toggleFullScreenRequested);
                sessionPage->toggleFullScreenRequested();
                REQUIRE(spy.size() == 1);
            }

            SECTION("should close session page when requested") {
                sessionPage->closeSessionRequested();
                REQUIRE(page->layout()->indexOf(sessionPage) < 0);
            }
        }

        SECTION("should display last resumed session") {
            emptyPage->lastSessionResumed();
            auto sessionPage = page->findChild<SessionPage *>();
            REQUIRE(sessionPage != nullptr);

            SECTION("should fire full screen request") {
                QSignalSpy spy(page.get(), &ClubPage::toggleFullScreenRequested);
                sessionPage->toggleFullScreenRequested();
                REQUIRE(spy.size() == 1);
            }

            SECTION("should close session page when requested") {
                sessionPage->closeSessionRequested();
                REQUIRE(page->layout()->indexOf(sessionPage) < 0);
            }
        }
    }
}