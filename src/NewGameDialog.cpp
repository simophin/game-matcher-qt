//
// Created by Fanchao Liu on 1/07/20.
//

#include "NewGameDialog.h"
#include "ui_NewGameDialog.h"

#include "ClubRepository.h"
#include "GameMatcher.h"
#include "NameFormatUtils.h"
#include "ToastDialog.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

static const auto dataRoleUserId = Qt::UserRole + 1;
static const auto dataRoleUserIsPaused = Qt::UserRole + 2;

static const SettingKey settingsKeyDurationSeconds = QStringLiteral("last_game_duration");
static const auto defaultGameDurationSeconds = 15 * 60;
static const auto minGameDurationSeconds = 30;

struct NewGameDialog::Impl {
    SessionId const session;
    ClubRepository *const repo;
    Ui::NewGameDialog ui;
    QSet<MemberId> temporarilyPaused;

    int countNonPausedPlayers() const {
        int rc = 0;
        for (int i = 0, size = ui.playerList->count(); i < size; i++) {
            if (!ui.playerList->item(i)->data(dataRoleUserIsPaused).toBool()) {
                rc++;
            }
        }
        return rc;
    }

    void updateDuration(uint64_t durationSeconds) {
        auto hours = durationSeconds / 3600;
        auto minutes = (durationSeconds - hours * 3600) / 60;
        auto seconds = durationSeconds - hours * 3600 - minutes * 60;
        ui.hourBox->setValue(hours);
        ui.minuteBox->setValue(minutes);
        ui.secondBox->setValue(seconds);
    }

    uint64_t readDurationSeconds() {
        return ui.hourBox->value() * 3600 + ui.minuteBox->value() * 60 + ui.secondBox->value();
    }
};

NewGameDialog::NewGameDialog(SessionId session, ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{session, repo}) {
    d->ui.setupUi(this);
    d->ui.playerList->setContextMenuPolicy(Qt::CustomContextMenu);

    d->updateDuration(repo->getSettingValue<uint64_t>(settingsKeyDurationSeconds).value_or(defaultGameDurationSeconds));

    connect(d->ui.hourBox,  qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);
    connect(d->ui.minuteBox,  qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);
    connect(d->ui.secondBox,  qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);

    refresh();
}

NewGameDialog::~NewGameDialog() {
    delete d;
}

void NewGameDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void NewGameDialog::refresh() {
    auto players = d->repo->getMembers(AllSession{d->session});
    formatMemberDisplayNames(players);

    QFont font;
    font.setPointSize(20);

    auto pausedFont = font;
    pausedFont.setStrikeOut(true);
    auto pausedForeground = QApplication::palette().mid();

    d->ui.playerList->clear();
    for (const auto &p : players) {
        auto widgetItem = new QListWidgetItem(p.displayName, d->ui.playerList);
        widgetItem->setData(dataRoleUserId, p.id);

        auto paused = p.status == Member::CheckedInPaused || d->temporarilyPaused.contains(p.id);
        widgetItem->setData(dataRoleUserIsPaused, paused);
        widgetItem->setFont(paused ? pausedFont : font);
        if (paused) {
            widgetItem->setForeground(pausedForeground);
        }
    }

    validateForm();
}

void NewGameDialog::on_playerList_customContextMenuRequested(const QPoint &pt) {
    if (auto item = d->ui.playerList->itemAt(pt)) {
        item->setSelected(true);

        auto menu = new QMenu(tr("Player option: "), d->ui.playerList);
        auto action = menu->addAction(tr("Check out"));
        auto memberId = item->data(dataRoleUserId).value<MemberId>();
        connect(action, &QAction::triggered, [=] {
            if (QMessageBox::question(this, tr("Check out"),
                                      tr("Are you sure to check out %1?").arg(item->text())) == QMessageBox::Yes) {
                if (d->repo->checkOut(d->session, memberId)) {
                    refresh();
                }
            }
        });

        if (!item->data(dataRoleUserIsPaused).toBool()) {
            action = menu->addAction(tr("Pause for 1 game"));
            connect(action, &QAction::triggered, [=] {
                d->temporarilyPaused.insert(memberId);
                refresh();
            });

            action = menu->addAction(tr("Pause until further notice"));
            connect(action, &QAction::triggered, [=] {
                d->temporarilyPaused.remove(memberId);
                if (d->repo->setPaused(d->session, memberId, true)) {
                    refresh();
                }
            });
        } else {
            action = menu->addAction(tr("Resume"));
            connect(action, &QAction::triggered, [=] {
                d->temporarilyPaused.remove(memberId);
                d->repo->setPaused(d->session, memberId, false);
                refresh();
            });
        }

        menu->popup(d->ui.playerList->mapToGlobal(pt));
    }
}

void NewGameDialog::on_playerList_itemDoubleClicked(QListWidgetItem *item) {
    auto memberId = item->data(dataRoleUserId).value<MemberId>();
    bool isPausing = !item->data(dataRoleUserIsPaused).toBool();
    if (isPausing) {
        d->temporarilyPaused.insert(memberId);
        if (auto member = d->repo->getMember(memberId)) {
            ToastDialog::show(tr("%1 paused for 1 game").arg(member->fullName()), 1000);
        }
    } else {
        d->temporarilyPaused.remove(memberId);
        d->repo->setPaused(d->session, memberId, false);
        if (auto member = d->repo->getMember(memberId)) {
            ToastDialog::show(tr("%1 resumes playing").arg(member->fullName()), 1000);
        }
    }
    
    refresh();
}

void NewGameDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        auto session = d->repo->getSession(d->session);
        if (!session) {
            button->setEnabled(false);
            button->setText(tr("Error getting session info"));
        } else if (d->countNonPausedPlayers() < session->session.numPlayersPerCourt) {
            button->setEnabled(false);
            button->setText(tr("Not enough people to play"));
        } else if (d->readDurationSeconds() < minGameDurationSeconds) {
            button->setEnabled(false);
            button->setText(tr("Game duration is too short"));
        }
        else {
            button->setEnabled(true);
            button->setText(tr("Start"));
        }
    }
}


void NewGameDialog::accept() {
    if (auto session = d->repo->getSession(d->session)) {
        QVector<CourtId> courtIds;
        courtIds.reserve(session->courts.size());
        for (const auto &court : session->courts) {
            courtIds.push_back(court.id);
        }
        
        auto progressDialog = new QProgressDialog(tr("Calculating..."), tr("Cancel"), 0, 0, this);
        progressDialog->open();

        auto resultWatcher = new QFutureWatcher<std::vector<GameAllocation>>(this);
        connect(resultWatcher, &QFutureWatcherBase::finished, [=] {
            progressDialog->close();

            if (d->repo->createGame(d->session, resultWatcher->result(), d->readDurationSeconds())) {
                emit this->newGameMade();
                QDialog::accept();
                return;
            }

            resultWatcher->deleteLater();
        });

        QVector<Member> eligiblePlayers;
        for (const auto &m : d->repo->getMembers(CheckedIn{d->session, false})) {
            if (!d->temporarilyPaused.contains(m.id)) {
                eligiblePlayers.append(m);
            }
        }

        auto levelRange = d->repo->getLevelRange();
        auto pastAllocations = d->repo->getPastAllocations(d->session);

        resultWatcher->setFuture(
                QtConcurrent::run([=] {
                    return GameMatcher::match(pastAllocations,
                                              eligiblePlayers,
                                              courtIds,
                                              session->session.numPlayersPerCourt,
                                              levelRange.first, levelRange.second,
                                              QDateTime::currentMSecsSinceEpoch());
                })
        );

        return;
    }

    QMessageBox::critical(this, tr("Fail"), tr("Unable to create a new game"));
}
