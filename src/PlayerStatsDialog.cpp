//
// Created by Fanchao Liu on 3/08/20.
//

#include "PlayerStatsDialog.h"
#include "ui_PlayerStatsDialog.h"

#include "ClubRepository.h"
#include "MemberPainter.h"

#include <QEvent>

struct PlayerStatsDialog::Impl {
    MemberId const memberId;
    SessionId const sessionId;
    ClubRepository *const repo;

    Ui::PlayerStatsDialog ui;
};

PlayerStatsDialog::PlayerStatsDialog(const BaseMember &m, SessionId sessionId, ClubRepository *club, QWidget *parent)
        : QDialog(parent), d(new Impl{m.id, sessionId, club}) {
    d->ui.setupUi(this);

    setWindowTitle(tr("Game statistics for %1").arg(m.fullName()));

    reload();
}

PlayerStatsDialog::~PlayerStatsDialog() {
    delete d;
}

void PlayerStatsDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void PlayerStatsDialog::reload() {
    auto stats = d->repo->getMemberGameStats(d->memberId, d->sessionId);
    d->ui.numGamesValue->setText(QString::number(stats.pastGames.size()));
    d->ui.numGamesOffValue->setText(QString::number(stats.numGamesOff));

    d->ui.pastGameTable->setRowCount(stats.pastGames.size());
    d->ui.pastGameTable->setColumnCount(3);

    QFont font;
    font.setPointSizeF(20.0);

    for (int i = 0, size = stats.pastGames.size(); i < size; i++) {
        const auto &pastGame = stats.pastGames[i];
        int j = 0;

        d->ui.pastGameTable->setItem(i, j++, new QTableWidgetItem(pastGame.startTime.time().toString()));
        d->ui.pastGameTable->setItem(i, j++, new QTableWidgetItem(pastGame.courtName));
        d->ui.pastGameTable->setItem(i, j++, new QTableWidgetItem(QString::number(pastGame.quality)));

        for (const auto &player : pastGame.players) {
            if (player.id != d->memberId) {
                if (d->ui.pastGameTable->columnCount() < j + 1) {
                    d->ui.pastGameTable->setColumnCount(j + 1);
                }

                auto item = new QTableWidgetItem(player.fullName());
                item->setFont(font);
                item->setForeground(MemberPainter::colorForMember(player));
                d->ui.pastGameTable->setItem(i, j++, item);
            }
        }
    }

    QStringList headers = {
            tr("Start time"),
            tr("Court"),
            tr("Match quality"),
    };

    for (size_t i = 0, size = d->ui.pastGameTable->columnCount() - headers.size(); i < size; i++) {
        headers.append(tr("Player %1").arg(i + 1));
    }

    d->ui.pastGameTable->setHorizontalHeaderLabels(headers);
    d->ui.pastGameTable->resizeColumnsToContents();
}
