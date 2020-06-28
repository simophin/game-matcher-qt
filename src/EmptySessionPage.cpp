//
// Created by Fanchao Liu on 28/06/20.
//

#include "EmptySessionPage.h"
#include "ui_EmptySessionPage.h"

#include "clubrepository.h"

struct EmptySessionPage::Impl {
    ClubRepository * const repo;
    Ui::EmptySessionPage ui;
};

EmptySessionPage::EmptySessionPage(ClubRepository *repo, QWidget *parent)
        : QFrame(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    applyInfo();
    connect(d->repo, &ClubRepository::clubInfoChanged, this, &EmptySessionPage::applyInfo);
    connect(d->repo, &ClubRepository::lastGameInfoChanged, this, &EmptySessionPage::applyInfo);
}

EmptySessionPage::~EmptySessionPage() {
    delete d;
}

void EmptySessionPage::applyInfo() {
    d->ui.clubNameLabel->setText(tr("Welcome to %1").arg(d->repo->clubInfo().name));
    d->ui.resumeButton->setEnabled(d->repo->getLastSession().has_value());
}
