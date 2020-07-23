//
// Created by Fanchao Liu on 27/06/20.
//
#include <QMessageBox>
#include "SessionPage.h"
#include "ui_SessionPage.h"

#include "ClubRepository.h"
#include "Adapter.h"
#include "MemberSelectDialog.h"
#include "CheckInDialog.h"
#include "EditMemberDialog.h"
#include "NewGameDialog.h"
#include "CourtDisplay.h"
#include "ToastEvent.h"

#include <functional>
#include <QTimer>
#include <QMenu>
#include <QSoundEffect>
#include <QApplication>

static const auto alarmDurationSeconds = 15;

struct SessionPage::Impl {
    ClubRepository *repo;

    SessionData session;
    Ui::SessionPage ui;

    std::optional<GameInfo> lastGame;
    QTimer gameTimer = QTimer();
    QSoundEffect sound = QSoundEffect();
};

SessionPage::SessionPage(Impl *d, QWidget *parent)
        : QWidget(parent), d(d) {
    d->ui.setupUi(this);
    setWindowTitle(tr("%1 game session").arg(d->repo->getClubName()));

    d->sound.setSource(QUrl::fromLocalFile(QStringLiteral(":/sound/alarm_clock.wav")));
    d->sound.setLoopCount(QSoundEffect::Infinite);

    connect(d->repo, &ClubRepository::sessionChanged, [=](auto sessionId) {
        if (d->session.session.id == sessionId) {
            onCurrentGameChanged();
        }
    });

    d->gameTimer.setInterval(1000);
    d->gameTimer.setSingleShot(true);
    connect(&d->gameTimer, &QTimer::timeout, this, &SessionPage::updateElapseTime);

    onSessionDataChanged();
    onCurrentGameChanged();
}

SessionPage::~SessionPage() {
    delete d;
}

void SessionPage::onSessionDataChanged() {
}

void SessionPage::onCurrentGameChanged() {
    if (auto courtLayout = d->ui.courtLayout) {
        auto game = d->repo->getLastGameInfo(d->session.session.id);
        if (!game) {
            return;
        }

        d->lastGame = game;
        updateElapseTime();

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
    }
}


void SessionPage::on_checkInButton_clicked() {
    auto dialog = new MemberSelectDialog(NonCheckedIn{d->session.session.id}, true, false, d->repo, this);
    dialog->setWindowTitle(tr("Who is checking in?"));
    dialog->setAcceptButtonText(tr("Check in"));
    dialog->show();
    connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
        auto checkInDialog = new CheckInDialog(id, d->session.session.id, d->repo, this);
        checkInDialog->show();
    });
}

void SessionPage::on_checkOutButton_clicked() {
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

void SessionPage::on_pauseButton_clicked() {
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

void SessionPage::on_resumeButton_clicked() {
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

void SessionPage::on_updateButton_clicked() {
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

void SessionPage::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void SessionPage::updateElapseTime() {
    QString value;
    if (d->lastGame) {
        auto now = QDateTime::currentDateTimeUtc();
        auto startTime = d->lastGame->startDateTime();
        auto totalSeconds = std::abs(startTime.secsTo(now));
        auto hours = totalSeconds / 3600;
        auto minutes = (totalSeconds - hours * 3600) / 60;
        auto seconds = totalSeconds % 60;
        std::array<char, 24> buf;

        if (hours < 1) {
            snprintf(buf.data(), buf.size(), "%02lld:%02lld", minutes, seconds);
        } else {
            snprintf(buf.data(), buf.size(), "%lld:%02lld:%02lld", hours, minutes, seconds);
        }

        d->gameTimer.start();
        auto isTimeout = totalSeconds > d->lastGame->durationSeconds;

        value = QLatin1String(buf.data());

        if (isTimeout && (totalSeconds % 2 == 0)) {
            value = QStringLiteral("<font color=\"red\">%1</font>").arg(value);
        }

        if (isTimeout && totalSeconds < d->lastGame->durationSeconds + alarmDurationSeconds) {
            if (!d->sound.isPlaying()) d->sound.play();
        } else {
            d->sound.stop();
        }

    } else {
        value = tr("n/a");
        d->gameTimer.stop();
    }

    d->ui.timeLabel->setText(tr("Time since last game: %1").arg(value));
}


void SessionPage::on_wardenOptionButton_clicked() {
    auto wardenMenu = new QMenu(tr("Warden options"), this);
    auto action = wardenMenu->addAction(tr("Start a new game"));
    connect(action, &QAction::triggered, [=] {
        auto dialog = new NewGameDialog(d->session.session.id, d->repo, this);
        dialog->show();
        connect(dialog, &NewGameDialog::newGameMade, this, &SessionPage::onCurrentGameChanged);
    });

//    if (d->lastGameStarted.isValid() && std::abs(QDateTime::currentDateTimeUtc().secsTo(d->lastGameStarted)) < 60) {
//        action = wardenMenu->addAction(tr("Withdraw last game"));
//        connect(action, &QAction::triggered, [] {
//            //TODO:
//        });
//    }

    wardenMenu->popup(d->ui.wardenOptionButton->mapToGlobal(
            QPoint(d->ui.wardenOptionButton->width() / 2, d->ui.wardenOptionButton->height() / 2)));
}

SessionPage *SessionPage::create(SessionId id, ClubRepository *repo, QWidget *parent) {
    if (auto session = repo->getSession(id))
        return new SessionPage(new Impl{repo, *session}, parent);

    return nullptr;
}

SessionId SessionPage::sessionId() const {
    return d->session.session.id;
}
