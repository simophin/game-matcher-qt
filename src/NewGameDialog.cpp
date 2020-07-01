//
// Created by Fanchao Liu on 1/07/20.
//

#include "NewGameDialog.h"
#include "ui_NewGameDialog.h"

#include "ClubRepository.h"
#include "GameMatcher.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

static const auto dataRoleUserId = Qt::UserRole + 1;
static const auto dataRoleUserIsPaused = Qt::UserRole + 2;

struct NewGameDialog::Impl {
    SessionId const session;
    ClubRepository *const repo;
    Ui::NewGameDialog ui;

    int countNonPausedPlayers() const {
        int rc = 0;
        for (int i = 0, size = ui.playerList->count(); i < size; i++) {
            if (!ui.playerList->item(i)->data(dataRoleUserIsPaused).toBool()) {
                rc++;
            }
        }
        return rc;
    }
};

NewGameDialog::NewGameDialog(SessionId session, ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{session, repo}) {
    d->ui.setupUi(this);
    d->ui.playerList->setContextMenuPolicy(Qt::CustomContextMenu);

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
    auto nonPausedPeople = d->repo->getAllMembers(CheckedIn{d->session, false});
    auto pausedPeople = d->repo->getAllMembers(CheckedIn{d->session, true});
    d->ui.playerList->clear();
    QFont font;
    font.setPointSize(16);
    for (const auto &item : nonPausedPeople) {
        auto widgetItem = new QListWidgetItem(item.fullName(), d->ui.playerList);
        widgetItem->setData(dataRoleUserId, item.id);
        widgetItem->setData(dataRoleUserIsPaused, false);
        widgetItem->setFont(font);
    }

    font.setStrikeOut(true);
    QPalette palette;
    for (const auto &item : pausedPeople) {
        auto widgetItem = new QListWidgetItem(item.fullName(), d->ui.playerList);
        widgetItem->setData(dataRoleUserId, item.id);
        widgetItem->setData(dataRoleUserIsPaused, true);
        widgetItem->setFont(font);
        widgetItem->setForeground(palette.midlight());
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

        if (item->data(dataRoleUserIsPaused).toBool()) {
            action = menu->addAction(tr("Resume"));
            connect(action, &QAction::triggered, [=] {
                if (d->repo->setPaused(d->session, memberId, false)) {
                    refresh();
                }
            });
        } else {
            action = menu->addAction(tr("Pause"));
            connect(action, &QAction::triggered, [=] {
                if (d->repo->setPaused(d->session, memberId, true)) {
                    refresh();
                }
            });
        }
        menu->popup(d->ui.playerList->mapToGlobal(pt));
    }
}

void NewGameDialog::on_playerList_itemDoubleClicked(QListWidgetItem *item) {
    bool paused = !item->data(dataRoleUserIsPaused).toBool();
    if (d->repo->setPaused(d->session, item->data(dataRoleUserId).value<MemberId>(), paused)) {
        refresh();
    }
}

void NewGameDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        auto session = d->repo->getSession(d->session);
        if (!session) {
            button->setEnabled(false);
            button->setText(tr("Error getting session info"));
        }
        else if (d->countNonPausedPlayers() < session->session.numPlayersPerCourt) {
            button->setEnabled(false);
            button->setText(tr("Not enough people to play"));
        }
        else {
            button->setEnabled(true);
            button->setText(tr("Start"));
        }
    }
}



void NewGameDialog::accept() {
    if (auto session = d->repo->getSession(d->session)) {
        auto matcher = new GameMatcher(this);
        QVector<CourtId> courtIds;
        courtIds.reserve(session->courts.size());
        for (const auto &court : session->courts) {
            courtIds.push_back(court.id);
        }

        auto result = matcher->match(d->repo->getPastAllocations(d->session),
                       d->repo->getAllMembers(AllMembers{}),
                       d->repo->getAllPlayers(d->session),
                       courtIds,
                       session->session.numPlayersPerCourt,
                       QDateTime::currentMSecsSinceEpoch()
        );

        if (d->repo->createGame(d->session, result)) {
            QDialog::accept();
            return;
        }
    }

    QMessageBox::critical(this, tr("Fail"), tr("Unable to create a new game"));
}
