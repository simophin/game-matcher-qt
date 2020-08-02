//
// Created by Fanchao Liu on 26/07/20.
//

#include "MemberMenu.h"

#include "ClubRepository.h"

#include "CheckInDialog.h"
#include "ToastDialog.h"
#include "EditMemberDialog.h"

#include <QPoint>
#include <QMenu>
#include <QMessageBox>


static void checkout(QWidget *parent, ClubRepository *repo, SessionId sessionId, const Member &m) {
    if (QMessageBox::question(parent, QObject::tr("Checking out"),
            QObject::tr("Check out <i>%1</i>?").arg(m.fullName())) == QMessageBox::No) {
        return;
    }

    if (repo->checkOut(sessionId, m.id)) {
        ToastDialog::show(QObject::tr("%1 left the game").arg(m.fullName()));
    }
}

static void pausePlaying(QWidget *parent, ClubRepository *repo, SessionId sessionId, const Member &m) {
    if (repo->setPaused(sessionId, m.id, true)) {
        ToastDialog::show(QObject::tr("%1 paused until further action").arg(m.fullName()));
    }
}

static void resumePlaying(QWidget *parent, ClubRepository *repo, SessionId sessionId, const Member &m) {
    if (repo->setPaused(sessionId, m.id, false)) {
        ToastDialog::show(QObject::tr("%1 resumed playing").arg(m.fullName()));
    }
}

static void markPaid(QWidget *parent, ClubRepository *repo, SessionId sessionId, const Member &m) {
    if (repo->setPaid(sessionId, m.id, true)) {
        ToastDialog::show(QObject::tr("%1 marked as paid").arg(m.fullName()));
    }
}

static void markUnpaid(QWidget *parent, ClubRepository *repo, SessionId sessionId, const Member &m) {
    if (repo->setPaid(sessionId, m.id, false)) {
        ToastDialog::show(QObject::tr("%1 marked as unpaid").arg(m.fullName()));
    }
}

static void editUser(QWidget *parent, ClubRepository *repo, const Member &m) {
    auto dialog = new EditMemberDialog(repo, parent);
    dialog->setMember(m.id);
    dialog->show();
}

void MemberMenu::showAt(QWidget *parent,
                        ClubRepository *repo,
                        std::optional<SessionId> sessionId,
                        const Member &m,
                        const QPoint &globalPos,
                        QRect *itemRect) {
    auto menu = new QMenu(QObject::tr("Member options"), parent);
    auto status = m.status.isValid() ? m.status.value<Member::Status>() : Member::NotCheckedIn;
    switch (status) {
        case Member::NotCheckedIn:
        case Member::CheckedOut:
            if (sessionId) {
                QObject::connect(
                        menu->addAction(QObject::tr("Check in")),
                        &QAction::triggered,
                        [=] {
                            (new CheckInDialog(m.id, *sessionId, repo, parent))->show();
                        });
            }
            break;

        case Member::CheckedIn:
        case Member::CheckedInPaused:
            if (sessionId) {
                QObject::connect(
                        menu->addAction(QObject::tr("Check out")),
                        &QAction::triggered,
                        [=] {
                            checkout(parent, repo, *sessionId, m);
                        });
            }
    }

    if (status == Member::CheckedIn && sessionId) {
        QObject::connect(
                menu->addAction(QObject::tr("Pause playing")),
                &QAction::triggered,
                [=] {
                    pausePlaying(parent, repo, *sessionId, m);
                });
    } else if (status == Member::CheckedInPaused && sessionId) {
        QObject::connect(
                menu->addAction(QObject::tr("Resume playing")),
                &QAction::triggered,
                [=] {
                    resumePlaying(parent, repo, *sessionId, m);
                });
    }

    if (m.paid.isValid() && sessionId) {
        if (m.paid.toBool()) {
            QObject::connect(
                    menu->addAction(QObject::tr("Mark as unpaid")),
                    &QAction::triggered,
                    [=] {
                        markUnpaid(parent, repo, *sessionId, m);
                    });
        } else {
            QObject::connect(
                    menu->addAction(QObject::tr("Mark as paid")),
                    &QAction::triggered,
                    [=] {
                        markPaid(parent, repo, *sessionId, m);
                    });
        }
    }

    QObject::connect(
            menu->addAction(QObject::tr("Edit info")),
            &QAction::triggered,
            [=] {
               editUser(parent, repo, m);
            });

    menu->popup(globalPos);
}
