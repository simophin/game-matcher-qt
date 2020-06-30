//
// Created by Fanchao Liu on 27/06/20.
//
#include <QMessageBox>
#include "SessionWindow.h"
#include "CourtDisplay.h"
#include "ui_SessionWindow.h"

#include "ClubRepository.h"
#include "Adapter.h"
#include "MemberSelectDialog.h"
#include "CheckInDialog.h"
#include "EditMemberDialog.h"

#include <functional>

struct SessionWindow::Impl {
    ClubRepository *repo;
    SessionData session;
    Ui::SessionWindow ui;

    void selectMemberTo(QWidget *parent, const QString &action, MemberSearchFilter filter, std::function<bool(const Member &)> cb) {
        auto dialog = new MemberSelectDialog(filter, repo, parent);
        dialog->setWindowTitle(tr("Select yourself to %1...").arg(action));
        dialog->show();

        connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
            auto member = repo->getMember(id);
            if (!member) {
                QMessageBox::critical(parent, tr("Error"), tr("Unable to find given member to %1").arg(action));
                return;

            }
            if (QMessageBox::question(parent, tr("Check out"), tr("Are you sure to %1 %2").arg(action, member->fullName())) ==
                QMessageBox::Yes) {
                if (cb(*member)) {
                    QMessageBox::information(parent, tr("Success"), tr("%1 as %2: success").arg(action, member->fullName()));
                } else {
                    QMessageBox::critical(parent, tr("Error"),
                                          tr("Unable to %1 as %2. Maybe you have already done so.").arg(action, member->fullName()));
                }
            }
        });
    }
};

static const auto minCourtDisplayWidth = 90;
static const auto maxCourtDisplayWidth = 120;


SessionWindow::SessionWindow(ClubRepository *repo, SessionId sessionId, QWidget *parent)
        : QMainWindow(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    if (auto session = repo->getSession(sessionId)) {
        d->session = *session;
        setWindowTitle(tr("%1 game session").arg(repo->clubInfo().name));
        onSessionDataChanged();
        onCurrentGameChanged();
    } else {
        QMessageBox::warning(this, tr("Unable to open session"), tr("Please try again"));
        close();
    }
}

SessionWindow::~SessionWindow() {
    delete d;
}

void SessionWindow::onSessionDataChanged() {
}

void SessionWindow::onCurrentGameChanged() {
    auto game = d->repo->lastGameInfo();
    if (game.empty()) {
    } else {
        QVector<CourtInfo> courts;
        courts.reserve(game.courts.size());
        QSet<MemberId> sameFirstNames;
        QMap<QString, MemberId> firstNameMaps;
        for (const auto &m : d->session.checkedIn) {
            if (firstNameMaps.contains(m.firstName)) {
                sameFirstNames.insert(m.id);
                sameFirstNames.insert(firstNameMaps[m.firstName]);
            } else {
                firstNameMaps[m.firstName] = m.id;
            }
        }

        for (const auto &item : game.courts) {
            CourtInfo info;
            info.courtName = item.courtName;
            info.courtId = item.courtId;
            for (const auto &m : item.players) {
                info.players.append(CourtPlayer{
                        m.id,
                        sameFirstNames.contains(m.id)
                        ? tr("%1 %2", "Full name: First Last").arg(m.firstName, m.lastName)
                        : m.firstName
                });
            }
        }

        setEntities(d->ui.courtGrid, courts,
                    [=] { return new CourtDisplay(this); },
                    [](CourtDisplay *display, const CourtInfo &info) {
                        display->setCourt(info);
                    });
    }
}



void SessionWindow::on_checkInButton_clicked() {
    auto dialog = new MemberSelectDialog(NonCheckedIn{d->session.session.id}, d->repo, this);
    dialog->setWindowTitle(tr("Select yourself to check in..."));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        auto checkInDialog = new CheckInDialog(id, d->session.session.id, d->repo, this);
        checkInDialog->show();
    });
}

void SessionWindow::on_checkOutButton_clicked() {
    d->selectMemberTo(this, tr("check out"), CheckedIn{d->session.session.id}, [=](const Member &m) {
        return d->repo->checkOut(d->session.session.id, m.id);
    });
}

void SessionWindow::on_pauseButton_clicked() {
    d->selectMemberTo(this, tr("pause"), CheckedIn{d->session.session.id, false}, [=](const Member &m) {
        return d->repo->setPaused(d->session.session.id, m.id, true);
    });
}

void SessionWindow::on_resumeButton_clicked() {
    d->selectMemberTo(this, tr("resume"), CheckedIn{d->session.session.id, true}, [=](const Member &m) {
        return d->repo->setPaused(d->session.session.id, m.id, false);
    });
}

void SessionWindow::on_registerButton_clicked() {
    auto dialog = new EditMemberDialog(d->repo, this);
    dialog->show();
    connect(dialog, &EditMemberDialog::newMemberCreated, [=](auto memberId) {
        if (QMessageBox::question(this, tr("Success"),
                tr("Register successfully. \nDo you want to check in for the game?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
            (new CheckInDialog(memberId, d->session.session.id, d->repo, this))->show();
        }
    });
}

void SessionWindow::on_updateButton_clicked() {
    auto dialog = new MemberSelectDialog(AllMembers{}, d->repo, this);
    dialog->setWindowTitle(tr("Select yourself to edit..."));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        auto editDialog = new EditMemberDialog(d->repo, this);
        editDialog->setMember(id);
        editDialog->show();
        connect(editDialog, &EditMemberDialog::memberUpdated, [=] {
            QMessageBox::information(this, tr("Success"), tr("Member information updated successfully"));
        });
    });

}
