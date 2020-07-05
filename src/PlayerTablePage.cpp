//
// Created by Fanchao Liu on 5/07/20.
//

#include "PlayerTablePage.h"
#include "ui_PlayerTablePage.h"

#include "ClubRepository.h"
#include "CollectionUtils.h"

#include <QEvent>
#include <map>

struct PlayerTablePage::Impl {
    const SessionId sessionId;
    ClubRepository *const repo;
    Ui::PlayerTablePage ui;
};

PlayerTablePage::PlayerTablePage(SessionId id, ClubRepository *repo, QWidget *parent)
        : QWidget(parent), d(new Impl {id, repo}) {
    d->ui.setupUi(this);
    reload();
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
    auto allocations = d->repo->getPastAllocations(d->sessionId);
    auto members = d->repo->getMembers(AllSession{d->sessionId});
    auto memberById = associateBy<QHash<MemberId, Member>>(members, [](const Member &m) {
        return m.id;
    });
    QHash<MemberId, QVector<GameAllocation>> allocationByMemberId;
    std::map<GameId, int> gameColumns;
    for (const auto &allocation : allocations) {
        gameColumns[allocation.gameId] = 0;
        allocationByMemberId[allocation.memberId].append(allocation);
    }

    if (!gameColumns.empty()) {
        int i = 1;
        for (auto &entries : gameColumns) {
            entries.second = i++;
        }
    }

    std::sort(members.begin(), members.end(), [](const Member &a, const Member &b) {
        return a.fullName().localeAwareCompare(b.fullName());
    });

    for (int i = 0; i < members.size(); i++) {
        d->ui.table->setItem(i + 1, 0, new QTableWidgetItem(members[i].fullName()));
        if (auto found = allocationByMemberId.constFind(members[i].id); found != allocationByMemberId.constEnd()) {
            for (auto alloc : *found) {
                d->ui.table->setItem(i + 1, gameColumns[alloc.gameId],
                        new QTableWidgetItem(QString::number(alloc.courtId)));
            }
        }
    }
}
