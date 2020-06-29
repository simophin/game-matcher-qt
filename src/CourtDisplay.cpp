//
// Created by Fanchao Liu on 29/06/20.
//

#include "CourtDisplay.h"
#include "ui_CourtDisplay.h"


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

    const auto numViews = d->ui.memberListLayout->count();
    const auto numPlayers = court.players.size();
    const auto numReuse = qMin(numPlayers, numViews);
    int i = 0;
    for (; i < numReuse; i++) {
        qobject_cast<QLabel *>(d->ui.memberListLayout->itemAt(i)->widget())->setText(court.players[i].displayName);
    }
    
    if (numReuse < numPlayers) {
        // Add missing
        for (; i < numPlayers; i++) {
            d->ui.memberListLayout->addWidget(new QLabel(court.players[i].displayName, this));
        }
    } else if (numReuse < numViews) {
        // Remove excessive
        while (d->ui.memberListLayout->count() > numPlayers) {
            d->ui.memberListLayout->removeItem(d->ui.memberListLayout->itemAt(d->ui.memberListLayout->count() - 1));
        }
    }
}
