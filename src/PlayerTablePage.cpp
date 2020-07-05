//
// Created by Fanchao Liu on 5/07/20.
//

#include "PlayerTablePage.h"
#include "ui_PlayerTablePage.h"

#include "ClubRepository.h"
#include "CollectionUtils.h"

#include <QEvent>
#include <map>
#include <set>

struct PlayerTablePage::Impl {
    const SessionId sessionId;
    ClubRepository *const repo;
    Ui::PlayerTablePage ui;
};

PlayerTablePage::PlayerTablePage(SessionId id, ClubRepository *repo, QWidget *parent)
        : QWidget(parent), d(new Impl {id, repo}) {
    d->ui.setupUi(this);
    reload();
    connect(repo, &ClubRepository::sessionChanged, this, &PlayerTablePage::reload);
}

PlayerTablePage::~PlayerTablePage() {
    delete d;
}

void PlayerTablePage::changeEvent(QEvent *evt) {
    QWidget::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void PlayerTablePage::reload() {
    d->ui.table->clear();
    auto members = d->repo->getMembers(AllSession{d->sessionId});
    QHash<MemberId, QHash<GameId, CourtId>> allocationMap;
    std::set<GameId> gameIdSet;
    for (const auto &allocation : d->repo->getPastAllocations(d->sessionId)) {
        gameIdSet.insert(allocation.gameId);
        allocationMap[allocation.memberId][allocation.gameId] = allocation.courtId;
    }
    std::vector<GameId> gameIds(gameIdSet.begin(), gameIdSet.end());

    std::sort(members.begin(), members.end(), [](const Member &a, const Member &b) {
        return a.fullName().localeAwareCompare(b.fullName()) < 0;
    });

    d->ui.table->setRowCount(members.size() + 1);
    d->ui.table->setColumnCount(gameIds.size() + 1);

    for (int i = 0; i < members.size(); i++) {
        d->ui.table->setItem(i + 1, 0, new QTableWidgetItem(members[i].fullName()));
        auto &memberGames = allocationMap[members[i].id];
        for (int j = 0; j < gameIds.size(); j++) {
            auto gameId = gameIds[j];
            if (auto courtId = memberGames.constFind(gameId); courtId != memberGames.constEnd()) {
                d->ui.table->setItem(i + 1, j + 1, new QTableWidgetItem(QString::number(*courtId)));
            } else {
                d->ui.table->setItem(i + 1, j + 1, new QTableWidgetItem(tr("-")));
            }
        }

    }

    d->ui.table->resizeColumnsToContents();
    d->ui.table->resizeRowsToContents();
}
