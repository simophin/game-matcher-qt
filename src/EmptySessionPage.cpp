//
// Created by Fanchao Liu on 28/06/20.
//

#include "EmptySessionPage.h"
#include "ui_EmptySessionPage.h"

#include "clubrepository.h"
#include "NewSessionDialog.h"
#include "EditMemberDialog.h"

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
    connect(d->repo, &ClubRepository::lastSessionChanged, this, &EmptySessionPage::applyInfo);
}

EmptySessionPage::~EmptySessionPage() {
    delete d;
}

void EmptySessionPage::applyInfo() {
    d->ui.clubNameLabel->setText(tr("Welcome to %1").arg(d->repo->clubInfo().name));
    d->ui.resumeButton->setEnabled(d->repo->getLastSession().has_value());
}

void EmptySessionPage::on_startButton_clicked() {
    auto dialog = new NewSessionDialog(d->repo, this);
    dialog->show();
    connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
    connect(dialog, &NewSessionDialog::sessionCreated, this, &EmptySessionPage::newSessionCreated);
}

void EmptySessionPage::on_resumeButton_clicked() {
    emit lastSessionResumed();
}

void EmptySessionPage::on_updateButton_clicked() {
    //TODO
}

void EmptySessionPage::on_statsButton_clicked() {
    //TODO
}

void EmptySessionPage::on_newMemberButton_clicked() {
    auto dialog = new EditMemberDialog(d->repo, this);
    dialog->show();
}
