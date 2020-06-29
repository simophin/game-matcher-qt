//
// Created by Fanchao Liu on 29/06/20.
//

#include "CourtDisplay.h"
#include "ui_CourtDisplay.h"
#include "Adapter.h"


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

void CourtDisplay::setCourt(const CourtInfo &court) {
    d->ui.courtName->setText(court.courtName);

    setEntities(d->ui.memberListLayout,
                court.players,
                [=] { return new QLabel(this); },
                [](QLabel *label, const auto &player) {
                    label->setText(player.displayName);
                });
}
