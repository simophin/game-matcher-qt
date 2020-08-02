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
#include "LastSelectedCourts.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QCheckBox>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

static const auto dataRoleMember = Qt::UserRole;
static const auto propCourtId = "courtId";

static const SettingKey skLastSelectedCourts = QStringLiteral("last_selected_courts");
static const SettingKey skLastGameDurationSeconds = QStringLiteral("last_game_duration_seconds");

static const auto defaultGameDurationSeconds = 15 * 60;
static const auto minGameDurationSeconds = 30;

struct NewGameDialog::Impl {
    SessionData const session;
    ClubRepository *const repo;
    Ui::NewGameDialog ui;

    int countEligiblePlayer() const {
        int rc = 0;
        for (int i = 0, size = ui.playerList->count(); i < size; i++) {
            if (ui.playerList->item(i)->data(dataRoleMember).value<Member>().status == Member::CheckedIn) {
                rc++;
            }
        }
        return rc;
    }

    int courtCheckedCourts() const {
        int numCourts = 0;
        for (int i = 0, size = ui.courtBoxes->count(); i < size; i++) {
            if (auto checkbox = qobject_cast<QCheckBox *>(ui.courtBoxes->itemAt(i)->widget());
                    checkbox && checkbox->isChecked()) {
                numCourts++;
            }
        }
        return numCourts;
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

NewGameDialog::NewGameDialog(Impl *d, QWidget *parent)
        : QDialog(parent), d(d) {
    d->ui.setupUi(this);

    d->updateDuration(
            d->repo->getSettingValue<uint64_t>(skLastGameDurationSeconds).value_or(defaultGameDurationSeconds));

    connect(d->ui.hourBox, qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);
    connect(d->ui.minuteBox, qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);
    connect(d->ui.secondBox, qOverload<int>(&QSpinBox::valueChanged), this, &NewGameDialog::validateForm);

    connect(d->repo, &ClubRepository::sessionChanged, this, &NewGameDialog::refresh);

    connect(d->ui.playerList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item) {
        auto member = item->data(dataRoleMember).value<Member>();
        bool isPausing = item->data(dataRoleMember).value<Member>().status == Member::CheckedInPaused;
        d->repo->setPaused(d->session.session.id, member.id, !isPausing);
    });

    connect(d->ui.playerList, &QWidget::customContextMenuRequested, [=](QPoint pt) {
        if (auto item = d->ui.playerList->itemAt(pt)) {
            MemberMenu::showAt(this, d->repo, d->session.session.id,
                               item->data(dataRoleMember).value<Member>(),
                               d->ui.playerList->mapToGlobal(pt));
        }
    });

    auto lastSelectedCourts = LastSelectedCourt::fromString(d->repo->getSettingValue<QString>(skLastSelectedCourts).value_or(QString()));
    if (lastSelectedCourts && lastSelectedCourts->sessionId != d->session.session.id) {
        lastSelectedCourts.reset();
    }

    for (const auto &court : d->session.courts) {
        auto checkBox = new QCheckBox();
        d->ui.courtBoxes->addWidget(checkBox, 1);
        checkBox->setText(court.name);
        checkBox->setProperty(propCourtId, court.id);
        checkBox->setChecked(!lastSelectedCourts || (lastSelectedCourts->selectedCourts.contains(court.id)));
        connect(checkBox, &QCheckBox::stateChanged, this, &NewGameDialog::validateForm);
    }

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
    auto players = d->repo->getMembers(CheckedIn{d->session.session.id});
    formatMemberDisplayNames(players);

    QFont font;
    font.setPointSizeF(20.0);
    font.setBold(true);

    auto pausedForeground = QApplication::palette().mid();

    d->ui.playerList->clear();
    for (const auto &p : players) {
        auto widgetItem = new QListWidgetItem(p.displayName, d->ui.playerList);
        widgetItem->setData(dataRoleMember, QVariant::fromValue(p));

        auto paused = p.status == Member::CheckedInPaused;
        if (paused) font.setStrikeOut(true);
        if (p.paid == false) font.setUnderline(true);

        widgetItem->setFont(font);

        font.setStrikeOut(false);
        font.setUnderline(false);

        if (paused) {
            widgetItem->setForeground(pausedForeground);
        } else {
            widgetItem->setForeground(MemberPainter::colorForMember(p));
        }
    }

    validateForm();
}


void NewGameDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        auto numCourts = d->courtCheckedCourts();
        if (numCourts == 0) {
            button->setEnabled(false);
            button->setText(tr("Not court selected"));
        } else if (d->countEligiblePlayer() < d->session.session.numPlayersPerCourt * numCourts) {
            button->setEnabled(false);
            button->setText(tr("Not enough people to play"));
        } else if (d->readDurationSeconds() < minGameDurationSeconds) {
            button->setEnabled(false);
            button->setText(tr("Game duration is too short"));
        } else {
            button->setEnabled(true);
            button->setText(tr("Start"));
        }
    }
}


void NewGameDialog::accept() {
    QVector<CourtId> courtIds;
    for (int i = 0, size = d->ui.courtBoxes->count(); i < size; i++) {
        if (auto checkbox = qobject_cast<QCheckBox *>(d->ui.courtBoxes->itemAt(i)->widget());
                checkbox && checkbox->isChecked()) {
            courtIds.push_back(checkbox->property(propCourtId).value<CourtId>());
        }
    }

    auto progressDialog = new QProgressDialog(tr("Calculating..."), tr("Cancel"), 0, 0, this);
    progressDialog->open();

    auto resultWatcher = new QFutureWatcher<std::vector<GameAllocation>>(this);
    connect(resultWatcher, &QFutureWatcherBase::finished, [=] {
        progressDialog->close();

        if (d->repo->createGame(d->session.session.id, resultWatcher->result(), d->readDurationSeconds())) {
            emit this->newGameMade();
            QDialog::accept();
            return;
        }

        resultWatcher->deleteLater();
    });

    QVector<Member> eligiblePlayers;
    for (int i = 0, size = d->ui.playerList->count(); i < size; i++) {
        auto member = d->ui.playerList->item(i)->data(dataRoleMember).value<Member>();
        if (member.status == Member::CheckedIn) {
            eligiblePlayers.push_back(member);
        }
    }

    auto levelRange = d->repo->getLevelRange();
    auto pastAllocations = d->repo->getPastAllocations(d->session.session.id);
    size_t numPlayersPerCourt = d->session.session.numPlayersPerCourt;

    resultWatcher->setFuture(
            QtConcurrent::run([pastAllocations, eligiblePlayers, courtIds, numPlayersPerCourt, levelRange] {
                return GameMatcher::match(pastAllocations,
                                          eligiblePlayers,
                                          courtIds,
                                          numPlayersPerCourt,
                                          levelRange.first, levelRange.second,
                                          QDateTime::currentMSecsSinceEpoch());
            })
    );

    LastSelectedCourt lastSelected = { d->session.session.id };
    for (auto courtId : courtIds) {
        lastSelected.selectedCourts.insert(courtId);
    }

    if (!d->repo->saveSetting(skLastSelectedCourts, lastSelected.toString())) {
        qWarning() << "Unable to save last selected courts";
    }

    if (!d->repo->saveSetting(skLastGameDurationSeconds, QVariant::fromValue(d->readDurationSeconds()))) {
        qWarning() << "Unable to save last game duration seconds";
    }
}

NewGameDialog *NewGameDialog::create(SessionId id, ClubRepository *repo, QWidget *parent) {
    if (auto session = repo->getSession(id)) {
        return new NewGameDialog(new Impl{*session, repo}, parent);
    }

    return nullptr;
}
