//
// Created by Fanchao Liu on 29/06/20.
//

#include "CourtDisplay.h"
#include "ui_CourtDisplay.h"
#include "Adapter.h"

#include "ClubRepository.h"


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
                    return label; },
                [](QLabel *label, const Member &player) {
                    label->setText(player.displayName.isEmpty() ? player.fullName() : player.displayName);
                });
}
