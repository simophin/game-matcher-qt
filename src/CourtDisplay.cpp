//
// Created by Fanchao Liu on 29/06/20.
//

#include "CourtDisplay.h"
#include "ui_CourtDisplay.h"
#include "Adapter.h"

#include "ClubRepository.h"
#include "MemberPainter.h"

struct CourtDisplay::Impl {
    Ui::CourtDisplay ui;
};

CourtDisplay::CourtDisplay(QWidget *parent)
        : QWidget(parent), d(new Impl) {
    d->ui.setupUi(this);
}

CourtDisplay::~CourtDisplay() {
    delete d;
}

void CourtDisplay::setCourt(const CourtPlayers &court) {
    d->ui.courtName->setText(court.courtName);

    setEntities(d->ui.memberListLayout,
                court.players,
                [=] {
                    auto label = new QLabel(this);
                    label->setProperty("isMember", true);
                    label->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(label, &QLabel::customContextMenuRequested, [=](QPoint pos) {
                        label->setFocus(Qt::PopupFocusReason);
                        emit this->memberRightClicked(label->property("member").value<Member>(), label->mapToGlobal(pos));
                    });
                    return label; },
                [](QLabel *label, const Member &player) {
                    label->setText(player.displayName.isEmpty() ? player.fullName() : player.displayName);
                    auto palette = label->palette();
                    auto color = MemberPainter::colorForMember(player);
                    palette.setColor(QPalette::Text, color);
                    palette.setColor(QPalette::ButtonText, color);
                    palette.setColor(QPalette::WindowText, color);
                    label->setProperty("member", QVariant::fromValue(player));
                    label->setPalette(palette);
                });
}
