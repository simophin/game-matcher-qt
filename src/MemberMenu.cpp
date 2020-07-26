//
// Created by Fanchao Liu on 26/07/20.
//

#include "MemberMenu.h"

#include "ClubRepository.h"

#include "CheckInDialog.h"

#include <QPoint>
#include <QMenu>

static void showCheckOutConfirmation(ClubRepository *repo, SessionId sessionId, const Member &m) {

}

static void showPausePlaying(ClubRepository *repo, SessionId sessionId, const Member &m) {

}

static void showResumePlaying(ClubRepository *repo, SessionId sessionId, const Member &m) {

}

static void markPaid(ClubRepository *repo, SessionId sessionId, const Member &m) {

}

static void markUnpaid(ClubRepository *repo, SessionId sessionId, const Member &m) {

}

void MemberMenu::showAt(QWidget *parent,
                        ClubRepository *repo,
                        SessionId sessionId,
                        const Member &m,
                        const QPoint &globalPos) {
    auto menu = new QMenu(QObject::tr("Member options"), parent);
    auto status = m.status.isValid() ? m.status.value<Member::Status>() : Member::NotCheckedIn;
    switch (status) {
        case Member::NotCheckedIn:
        case Member::CheckedOut:
            QObject::connect(
                    menu->addAction(QObject::tr("Check in")),
                    &QAction::triggered,
                    [=] {
                        (new CheckInDialog(m.id, sessionId, repo))->show();
                    });
            break;

        case Member::CheckedIn:
        case Member::CheckedInPaused:
            QObject::connect(
                    menu->addAction(QObject::tr("Check out")),
                    &QAction::triggered,
                    [=] {
                        showCheckOutConfirmation(repo, sessionId, m);
                    });
    }

    if (status == Member::CheckedIn) {
        QObject::connect(
                menu->addAction(QObject::tr("Pause playing")),
                &QAction::triggered,
                [=] {
                    showPausePlaying(repo, sessionId, m);
                });
    } else if (status == Member::CheckedInPaused) {
        QObject::connect(
                menu->addAction(QObject::tr("Resume playing")),
                &QAction::triggered,
                [=] {
                    showResumePlaying(repo, sessionId, m);
                });
    }

    if (m.paid.isValid()) {
        if (m.paid.toBool()) {
            QObject::connect(menu->addAction(QObject::tr("Mark as unpaid")),
                             &QAction::triggered,
                             [=] {
                                 markUnpaid(repo, sessionId, m);
                             });
        } else {
            QObject::connect(menu->addAction(QObject::tr("Mark as paid")),
                             &QAction::triggered,
                             [=] {
                                 markPaid(repo, sessionId, m);
                             });
        }
    }

    menu->popup(globalPos);
}
