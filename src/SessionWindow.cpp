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
#include "ToastEvent.h"
#include "ToastDialog.h"

#include <functional>
#include <QTimer>
#include <QMenu>

struct SessionWindow::Impl {
    ClubRepository *repo;
    ToastDialog *toastDialog;
    SessionData session;
    Ui::SessionWindow ui;

    QDateTime lastGameStarted;
    QTimer gameTimer = QTimer();
};

SessionWindow::SessionWindow(ClubRepository *repo, SessionId sessionId, QWidget *parent)
        : QMainWindow(parent), d(new Impl{repo, new ToastDialog(this)}) {
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

        connect(repo, &ClubRepository::sessionChanged, [=](auto sessionId) {
            if (d->session.session.id == sessionId) {
                onCurrentGameChanged();
            }
        });

        QCoreApplication::instance()->installEventFilter(this);
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

        auto startTime = QDateTime::fromSecsSinceEpoch(game->startTime);

        if (d->lastGameStarted != startTime) {
            d->lastGameStarted = startTime;
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
    auto dialog = new MemberSelectDialog(NonCheckedIn{d->session.session.id}, true, false, d->repo, this);
    dialog->setWindowTitle(tr("Who is checking in?"));
    dialog->setAcceptButtonText(tr("Check in"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        auto checkInDialog = new CheckInDialog(id, d->session.session.id, d->repo, this);
        checkInDialog->show();
    });
}

void SessionWindow::on_checkOutButton_clicked() {
    auto dialog = new MemberSelectDialog(CheckedIn{d->session.session.id}, false, true, d->repo, this);
    dialog->setWindowTitle(tr("Who is leaving the game?"));
    dialog->setAcceptButtonText(tr("Check out"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        if (auto member = d->repo->getMember(id)) {
            if (QMessageBox::question(this, tr("Confirm checking out"),
                                      tr("Are you sure to check out %1").arg(member->fullName())) == QMessageBox::Yes) {
                d->repo->checkOut(d->session.session.id, id);
                ToastEvent::show(tr("%1 checked out").arg(member->fullName()));
            }
        }
    });
}

void SessionWindow::on_pauseButton_clicked() {
    auto dialog = new MemberSelectDialog(CheckedIn{d->session.session.id, false}, false, true, d->repo, this);
    dialog->setWindowTitle(tr("Who is pausing?"));
    dialog->setAcceptButtonText(tr("Pause playing"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        if (auto member = d->repo->getMember(id)) {
            d->repo->setPaused(d->session.session.id, id, true);
            ToastEvent::show(tr("%1 paused playing").arg(member->fullName()));
        }
    });
}

void SessionWindow::on_resumeButton_clicked() {
    auto dialog = new MemberSelectDialog(CheckedIn{d->session.session.id, true}, false, true, d->repo, this);
    dialog->setWindowTitle(tr("Who is resuming?"));
    dialog->setAcceptButtonText(tr("Resume playing"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        if (auto member = d->repo->getMember(id)) {
            d->repo->setPaused(d->session.session.id, id, false);
            ToastEvent::show(tr("%1 resumed playing").arg(member->fullName()));
        }
    });
}

void SessionWindow::on_updateButton_clicked() {
    auto dialog = new MemberSelectDialog(AllMembers{}, false, true, d->repo, this);
    dialog->setWindowTitle(tr("Whom to update?"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        auto editDialog = new EditMemberDialog(d->repo, this);
        editDialog->setMember(id);
        editDialog->show();
        connect(editDialog, &EditMemberDialog::memberUpdated, [=] {
            ToastEvent::show(tr("Information updated"));
        });
    });

}

void SessionWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
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

bool SessionWindow::eventFilter(QObject *watched, QEvent *event) {
    if (auto toast = dynamic_cast<ToastEvent*>(event)) {
        d->toastDialog->showMessage(toast->msg(), toast->delayMills());
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void SessionWindow::on_wardenOptionButton_clicked() {
    auto wardenMenu = new QMenu(tr("Warden options"), this);
    auto action = wardenMenu->addAction(tr("Start a new game"));
    connect(action, &QAction::triggered, [=] {
        auto dialog = new NewGameDialog(d->session.session.id, d->repo, this);
        dialog->show();
        connect(dialog, &NewGameDialog::newGameMade, this, &SessionWindow::onCurrentGameChanged);
    });

    if (d->lastGameStarted.isValid() && std::abs(QDateTime::currentDateTimeUtc().secsTo(d->lastGameStarted)) < 60) {
        action = wardenMenu->addAction(tr("Withdraw last game"));
        connect(action, &QAction::triggered, [] {
            //TODO:
        });
    }
    wardenMenu->popup(d->ui.wardenOptionButton->mapToGlobal(
            QPoint(d->ui.wardenOptionButton->width() / 2, d->ui.wardenOptionButton->height() / 2)));
}
