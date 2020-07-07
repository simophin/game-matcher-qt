//
// Created by Fanchao Liu on 27/06/20.
//
#include <QMessageBox>
#include "SessionWindow.h"
#include "ui_SessionWindow.h"

#include "ClubRepository.h"
#include "Adapter.h"
#include "MemberSelectDialog.h"
#include "CheckInDialog.h"
#include "EditMemberDialog.h"
#include "NewGameDialog.h"
#include "CourtDisplay.h"

#include <functional>
#include <QPointer>
#include <QTimer>

struct SessionWindow::Impl {
    ClubRepository *repo;
    SessionData session;
    Ui::SessionWindow ui;

    QDateTime lastGameStarted;
    QTimer gameTimer = QTimer();

    void selectMemberTo(QWidget *parent, const QString &action, MemberSearchFilter filter,
                        std::function<bool(const Member &)> cb) {
        auto dialog = new MemberSelectDialog(filter, repo, parent);
        dialog->setWindowTitle(tr("Select yourself to %1...").arg(action));
        dialog->show();

        connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
            auto member = repo->getMember(id);
            if (!member) {
                QMessageBox::critical(parent, tr("Error"), tr("Unable to find given member to %1").arg(action));
                return;

            }
            if (QMessageBox::question(parent, tr("Check out"),
                                      tr("Are you sure to %1 %2").arg(action, member->fullName())) ==
                QMessageBox::Yes) {
                if (cb(*member)) {
                    QMessageBox::information(parent, tr("Success"),
                                             tr("%1 as %2: success").arg(action, member->fullName()));
                } else {
                    QMessageBox::critical(parent, tr("Error"),
                                          tr("Unable to %1 as %2. Maybe you have already done so.").arg(action,
                                                                                                        member->fullName()));
                }
            }
        });
    }
};

SessionWindow::SessionWindow(ClubRepository *repo, SessionId sessionId, QWidget *parent)
        : QMainWindow(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    d->ui.playerTable->load(sessionId, repo);

    d->gameTimer.setInterval(1000);
    d->gameTimer.setSingleShot(true);
    connect(&d->gameTimer, &QTimer::timeout, this, &SessionWindow::updateElapseTime);

    if (auto session = repo->getSession(sessionId)) {
        d->session = *session;
        setWindowTitle(tr("%1 game session").arg(repo->getClubInfo().name));
        onSessionDataChanged();
        onCurrentGameChanged();

        connect(repo, &ClubRepository::sessionChanged, this, &SessionWindow::onCurrentGameChanged);
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
    if (auto courtLayout = d->ui.courtLayout) {
        auto game = d->repo->getLastGameInfo(d->session.session.id);
        if (!game) {
            return;
        }

        if (d->lastGameStarted != game->startTime) {
            d->lastGameStarted = game->startTime;
            updateElapseTime();
        }

        auto createWidget = [this]() {
            return new CourtDisplay(this);
        };

        auto updateWidget = [this](CourtDisplay *widget, const CourtPlayers &court) {
            widget->setCourt(court);
        };

        setEntities(courtLayout, game->courts, createWidget, updateWidget);

        d->ui.benchList->clear();

        QFont itemFont;
        itemFont.setPointSize(18);

        for (const auto &item : game->waiting) {
            auto listItem = new QListWidgetItem(item.displayName, d->ui.benchList);
            listItem->setFont(itemFont);
        }

        statusBar()->showMessage(tr("Game started = %1").arg(game->id));
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

void SessionWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void SessionWindow::on_actionStartNewGame_triggered() {
    auto dialog = new NewGameDialog(d->session.session.id, d->repo, this);
    dialog->show();
    connect(dialog, &NewGameDialog::newGameMade, this, &SessionWindow::onCurrentGameChanged);
}

void SessionWindow::updateElapseTime() {
    QString value;
    if (d->lastGameStarted.isValid()) {
        auto now = QDateTime::currentDateTimeUtc();
        auto totalSeconds = std::abs(d->lastGameStarted.secsTo(now));
        auto hours = totalSeconds / 3600;
        auto minutes = (totalSeconds - hours * 3600) / 60;
        auto seconds = totalSeconds % 60;
        std::array<char, 24> buf;

        if (hours < 1) {
            snprintf(buf.data(), buf.size(), "%02lld:%02lld", minutes, seconds);
        } else {
            snprintf(buf.data(), buf.size(), "%lld:%02lld:%02lld", hours, minutes, seconds);
        }

        value = QLatin1String(buf.data());
        d->gameTimer.start();
    } else {
        value = tr("n/a");
        d->gameTimer.stop();
    }

    d->ui.timeLabel->setText(tr("Time since last game: %1").arg(value));
}
