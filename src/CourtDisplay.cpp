//
// Created by Fanchao Liu on 29/06/20.
//

#include "CourtDisplay.h"
#include "ui_CourtDisplay.h"
#include "Adapter.h"

#include "ClubRepository.h"
#include "MemberPainter.h"
#include "MemberLabel.h"

#include <QPainter>
#include <optional>
#include <cmath>

static const auto pkMember = "member";

struct CourtDisplay::Impl {
    Ui::CourtDisplay ui;
    QFont nameFont;
    std::optional<CourtPlayers> court;
};


CourtDisplay::CourtDisplay(QWidget *parent)
        : QWidget(parent), d(new Impl) {
    d->ui.setupUi(this);

    d->nameFont.setPointSize(40.0);
    d->nameFont.setBold(true);
}

CourtDisplay::~CourtDisplay() {
    delete d;
}

void CourtDisplay::setCourt(const CourtPlayers &court) {
    d->court = court;
    d->ui.courtName->setText(tr("Court %1").arg(court.courtName));

    if (size().isValid()) {
        applyData();
    }
}

void CourtDisplay::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    if (d->court && size().isValid()) {
        applyData();
    }
}

void CourtDisplay::applyData() {

    setEntities(d->ui.memberListLayout,
                d->court->players,
                [=] {
                    auto label = new MemberLabel(this);
                    label->setProperty("isMember", true);
                    label->setAlignment(Qt::AlignCenter);
                    label->setScaledContents(true);
                    label->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(label, &QLabel::customContextMenuRequested, [=](QPoint pos) {
                        emit this->memberRightClicked(label->property(pkMember).value<Member>(), label->mapToGlobal(pos));
                    });

                    return label; },
                [=](MemberLabel *label, const Member &player) {
                    label->setText(player.displayName.isEmpty() ? player.fullName() : player.displayName);

                    auto palette = label->palette();
                    auto color = MemberPainter::colorForMember(player);
                    palette.setColor(QPalette::Text, color);
                    palette.setColor(QPalette::ButtonText, color);
                    palette.setColor(QPalette::WindowText, color);
                    label->setProperty(pkMember, QVariant::fromValue(player));
                    label->setPalette(palette);
                    if (player.paid == false) {
                        d->nameFont.setUnderline(true);
                    }
                    d->nameFont.setStrikeOut(player.status == Member::CheckedOut || player.status == Member::CheckedInPaused);
                    label->setFont(d->nameFont);
                    d->nameFont.setUnderline(false);
                });
}
