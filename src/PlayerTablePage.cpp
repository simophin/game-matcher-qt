//
// Created by Fanchao Liu on 5/07/20.
//

#include "PlayerTablePage.h"
#include "ui_PlayerTablePage.h"

#include "ClubRepository.h"
#include "CollectionUtils.h"
#include "NameFormatUtils.h"

#include <QEvent>
#include <QMenu>
#include <map>
#include <set>

struct PlayerTablePage::Impl {
    SessionId sessionId = 0;
    ClubRepository *repo = nullptr;
    Ui::PlayerTablePage ui;
};

PlayerTablePage::PlayerTablePage(QWidget *parent)
        : QWidget(parent), d(new Impl) {
    d->ui.setupUi(this);
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
    if (!d->repo) return;

    d->ui.table->clear();
    auto members = d->repo->getMembers(AllSession{d->sessionId});
    formatMemberDisplayNames(members);

    auto courtById = associateBy<QHash<CourtId, Court>>(
            d->repo->getSession(d->sessionId)->courts,
            [](auto &c) {
                return c.id;
            });
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

    d->ui.table->setRowCount(members.size());
    d->ui.table->setColumnCount(gameIds.size() + 1);

    QFont memberFont;
    memberFont.setPointSize(18);
    
    auto courtFont = memberFont;
    memberFont.setBold(true);

    d->ui.table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Paid?")));
    for (int i = 1, size = d->ui.table->columnCount(); i < size; i++) {
        QTableWidgetItem *hdrItem;
        if (i == size - 1) {
            hdrItem = new QTableWidgetItem(QIcon(QStringLiteral(":images/down.png")), QString());
        } else {
            hdrItem = new QTableWidgetItem(QString());
        }
        d->ui.table->setHorizontalHeaderItem(i, hdrItem);
    }

    for (int i = 0; i < members.size(); i++) {
        const auto &member = members[i];
        auto memberItem = new QTableWidgetItem(member.displayName);
        auto status = member.status.value<Member::Status>();

        memberFont.setStrikeOut(status == Member::CheckedOut);
        memberItem->setFont(memberFont);
        memberItem->setData(Qt::UserRole, member.id);

        if (status == Member::CheckedOut || status == Member::CheckedInPaused) {
            memberItem->setForeground(QApplication::palette().mid());
        }

        const auto row = i;

        d->ui.table->setVerticalHeaderItem(row, memberItem);

        if (member.paid.isValid() && member.paid.toBool()) {
            auto checkMark = new QTableWidgetItem(QIcon(QStringLiteral(":images/checkmark.svg")), QString());
            d->ui.table->setItem(row, 0, checkMark);
        }

        auto &memberGames = allocationMap[member.id];
        for (int j = 0; j < gameIds.size(); j++) {
            auto gameId = gameIds[j];
                QTableWidgetItem *courtItem;
            if (auto courtId = memberGames.constFind(gameId); courtId != memberGames.constEnd()) {
                courtItem = new QTableWidgetItem(courtById[*courtId].name.trimmed());
            } else {
                courtItem = new QTableWidgetItem(tr("-"));
            }

            courtItem->setFont(courtFont);
            courtItem->setData(Qt::UserRole, member.id);
            d->ui.table->setItem(row, j + 1, courtItem);
        }

    }

    d->ui.table->resizeColumnsToContents();
}

void PlayerTablePage::on_table_customContextMenuRequested(const QPoint &pt) {
    if (auto item = d->ui.table->itemAt(pt); item) {
        d->ui.table->selectRow(item->row());
    }

    auto menu = new QMenu(tr("Player option"), d->ui.table);
    menu->addAction(tr("Pause"));
    menu->addAction(tr("Check out"));
    menu->addAction(tr("Change level"));
    menu->addAction(tr("Change name"));
    menu->addAction(tr("Mark as paid"));
    menu->popup(d->ui.table->mapToGlobal(pt));
}

void PlayerTablePage::load(SessionId id, ClubRepository *repo) {
    if (d->repo != repo) {
        if (d->repo) disconnect(d->repo, &ClubRepository::sessionChanged, this, &PlayerTablePage::reload);
        if (repo) {
            connect(repo, &ClubRepository::sessionChanged, this, &PlayerTablePage::reload);
        }
        d->repo = repo;
    }
    d->sessionId = id;
    reload();
}
