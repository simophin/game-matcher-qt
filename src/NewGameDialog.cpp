//
// Created by Fanchao Liu on 1/07/20.
//

#include "NewGameDialog.h"
#include "ui_NewGameDialog.h"

#include "ClubRepository.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>

static const auto dataRoleUserId = Qt::UserRole + 1;
static const auto dataRoleUserIsPaused = Qt::UserRole + 2;

struct NewGameDialog::Impl {
    SessionId const session;
    ClubRepository *const repo;
    Ui::NewGameDialog ui;
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
