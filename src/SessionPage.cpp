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
#include "NewGameDialog.h"
#include "CourtDisplay.h"
#include "MemberPainter.h"
#include "MemberMenu.h"
#include "CourtDisplayLayout.h"
#include "PlayerTablePage.h"
#include "PlayerStatsDialog.h"

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
    QLayout *courtLayout;

    std::optional<GameInfo> lastGame;
    QTimer gameTimer = QTimer();
    QSoundEffect sound = QSoundEffect();
};

SessionPage::SessionPage(Impl *d, QWidget *parent)
        : QWidget(parent), d(d) {
    d->ui.setupUi(this);
    d->ui.courtFrame->setLayout(d->courtLayout = new CourtDisplayLayout());

    d->ui.timeLabel->setFont(QFont(QStringLiteral("Noto Mono"), 21));

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

    onCurrentGameChanged();

    d->ui.benchList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->ui.benchList, &QListWidget::customContextMenuRequested, [=](QPoint pos) {
        if (auto item = d->ui.benchList->itemAt(pos)) {
            auto member = item->data(Qt::UserRole).value<Member>();
            d->ui.benchList->visualItemRect(item);
            showMemberMenuAt(member, d->ui.benchList->mapToGlobal(pos));
        }
    });

    connect(d->ui.benchList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item) {
        auto member = item->data(Qt::UserRole).value<Member>();
        (new PlayerStatsDialog(member, d->session.session.id, d->repo, this))->show();
    });

    connect(d->ui.checkInButton, &QPushButton::clicked, [=] {
        auto dialog = new MemberSelectDialog(NonCheckedIn{d->session.session.id}, true, false, d->repo, this);
        dialog->setWindowTitle(tr("Who is checking in?"));
        dialog->setAcceptButtonText(tr("Check in"));
        dialog->show();
        connect(dialog, &MemberSelectDialog::memberSelected, [=](MemberId id) {
            auto checkInDialog = new CheckInDialog(id, d->session.session.id, d->repo, dialog);
            connect(checkInDialog, &CheckInDialog::memberCheckedIn, [=] {
                dialog->clearFilter();
            });
            checkInDialog->show();
        });
    });

    connect(d->ui.fullScreenButton, &QPushButton::clicked, this, &SessionPage::toggleFullScreenRequested);

    connect(d->ui.wardenOptionButton, &QPushButton::clicked, [=] {
        auto wardenMenu = new QMenu(tr("Warden options"), this);
        auto action = wardenMenu->addAction(tr("Start a new game"));
        connect(action, &QAction::triggered, [=] {
            auto dialog = NewGameDialog::create(d->session.session.id, d->repo, this);
            if (!dialog) {
                QMessageBox::warning(this, tr("Error"), tr("Unable to open new game dialog"));
                return;
            }

            dialog->show();
            connect(dialog, &NewGameDialog::newGameMade, this, &SessionPage::onCurrentGameChanged);
        });


        auto adminMenu = wardenMenu->addMenu(tr("Admin"));

        connect(adminMenu->addAction(tr("Show player board")), &QAction::triggered,
                [=] {
                    auto dialog = new QDialog(this);
                    QVBoxLayout *layout;
                    dialog->setLayout(layout = new QVBoxLayout());
                    auto page = new PlayerTablePage();
                    layout->addWidget(page);
                    page->setWindowModality(Qt::ApplicationModal);
                    page->load(d->session.session.id, d->repo);

                    dialog->show();
                    dialog->setWindowTitle(tr("Player board"));
                    dialog->adjustSize();
                });

        if (d->lastGame && std::abs(QDateTime::currentDateTimeUtc().secsTo(d->lastGame->startDateTime())) < 60) {
            connect(adminMenu->addAction(tr("Withdraw last game")), &QAction::triggered, [=] {
                if (QMessageBox::question(this, tr("Withdrawing game"),
                                          tr("Are you sure to withdraw current game? A new arrangement may be completely different!")) == QMessageBox::Yes) {
                    d->repo->withdrawLastGame(d->session.session.id);
                }
            });
        }

        connect(adminMenu->addAction(tr("Close current session")), &QAction::triggered,
                this, &SessionPage::closeSessionRequested);


        wardenMenu->popup(d->ui.wardenOptionButton->mapToGlobal(
                QPoint(d->ui.wardenOptionButton->width() / 2, d->ui.wardenOptionButton->height() / 2)));
    });

    connect(d->ui.bellButton, &QPushButton::clicked, [=](bool checked) {
        if (checked && !d->sound.isPlaying()) {
            d->sound.play();
        } else if (!checked && d->sound.isPlaying()) {
            d->sound.stop();
        }
    });
}

SessionPage::~SessionPage() {
    d->sound.stop();
    delete d;
}

void SessionPage::onCurrentGameChanged() {
    d->lastGame = d->repo->getLastGameInfo(d->session.session.id);
    updateElapseTime();

    auto createWidget = [this]() {
        auto display = new CourtDisplay(this);
        connect(display, &CourtDisplay::memberRightClicked, this, &SessionPage::showMemberMenuAt);
        return display;
    };

    auto updateWidget = [this](CourtDisplay *widget, const CourtPlayers &court) {
        widget->setCourt(court);
    };

    setEntities(d->courtLayout, d->lastGame ? d->lastGame->courts : QVector<CourtPlayers>(), createWidget, updateWidget);

    QFont itemFont;
    itemFont.setPointSize(18);
    itemFont.setBold(true);

    d->ui.benchList->clear();
    for (const auto &item : (d->lastGame ? d->lastGame->waiting : d->repo->getMembers(CheckedIn{d->session.session.id}))) {
        auto listItem = new QListWidgetItem(item.displayName, d->ui.benchList);
        listItem->setForeground(MemberPainter::colorForMember(item));
        listItem->setData(Qt::UserRole, QVariant::fromValue(item));
        bool isPaused = item.status == Member::CheckedInPaused;
        if (item.paid == false) {
            itemFont.setUnderline(true);
        }
        listItem->setFont(itemFont);
        itemFont.setUnderline(false);
        if (isPaused) {
            listItem->setText(tr("%1 (paused)").arg(listItem->text()));
        }
    }
}

void SessionPage::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void SessionPage::updateElapseTime() {
    QString value;
    QPalette palette;

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
            static QColor red("red");
            palette.setColor(QPalette::WindowText, red);
        }

        if (isTimeout && totalSeconds < d->lastGame->durationSeconds + alarmDurationSeconds) {
            if (!d->sound.isPlaying()) d->sound.play();
        } else if (!d->ui.bellButton->isChecked()) {
            d->sound.stop();
        }

    } else {
        d->gameTimer.stop();
    }

    d->ui.timeLabel->setVisible(!value.isEmpty());
    d->ui.timeLabel->setText(value);
    d->ui.timeLabel->setPalette(palette);
}

SessionPage *SessionPage::create(SessionId id, ClubRepository *repo, QWidget *parent) {
    if (auto session = repo->getSession(id))
        return new SessionPage(new Impl{repo, *session}, parent);

    return nullptr;
}

SessionId SessionPage::sessionId() const {
    return d->session.session.id;
}

void SessionPage::showMemberMenuAt(const Member &m, const QPoint &pos) {
    MemberMenu::showAt(this, d->repo, d->session.session.id, m, pos);
}
