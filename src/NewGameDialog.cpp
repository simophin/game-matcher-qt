//
// Created by Fanchao Liu on 1/07/20.
//

#include "NewGameDialog.h"
#include "ui_NewGameDialog.h"

#include "ClubRepository.h"
#include "GameMatcher.h"
#include "NameFormatUtils.h"
#include "ToastDialog.h"
#include "MemberMenu.h"
#include "MemberPainter.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

static const auto dataRoleUserIsPaused = Qt::UserRole + 1;
static const auto dataRoleMember = Qt::UserRole + 2;

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
    connect(repo, &ClubRepository::sessionChanged, this, &NewGameDialog::refresh);
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
    auto players = d->repo->getMembers(CheckedIn{d->session});
    formatMemberDisplayNames(players);

    QFont font;
    font.setPointSize(20);

    auto pausedFont = font;
    pausedFont.setStrikeOut(true);
    auto pausedForeground = QApplication::palette().mid();

    d->ui.playerList->clear();
    for (const auto &p : players) {
        auto widgetItem = new QListWidgetItem(p.displayName, d->ui.playerList);
        widgetItem->setData(dataRoleMember, QVariant::fromValue(p));

        auto paused = p.status == Member::CheckedInPaused || d->temporarilyPaused.contains(p.id);
        widgetItem->setData(dataRoleUserIsPaused, paused);
        widgetItem->setFont(paused ? pausedFont : font);
        if (paused) {
            widgetItem->setForeground(pausedForeground);
        } else {
            widgetItem->setForeground(MemberPainter::colorForMember(p));
        }
    }

    validateForm();
}

void NewGameDialog::on_playerList_customContextMenuRequested(const QPoint &pt) {
    if (auto item = d->ui.playerList->itemAt(pt)) {
        MemberMenu::showAt(this, d->repo, d->session,
                item->data(dataRoleMember).value<Member>(),
                d->ui.playerList->mapToGlobal(pt));
    }
}

void NewGameDialog::on_playerList_itemDoubleClicked(QListWidgetItem *item) {
    auto member = item->data(dataRoleMember).value<Member>();
    bool isPausing = !item->data(dataRoleUserIsPaused).toBool();
    if (isPausing) {
        d->temporarilyPaused.insert(member.id);
        ToastDialog::show(tr("%1 paused for 1 game").arg(member.fullName()), 1000);
    } else {
        d->temporarilyPaused.remove(member.id);
        d->repo->setPaused(d->session, member.id, false);
        ToastDialog::show(tr("%1 resumes playing").arg(member.fullName()), 1000);
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
