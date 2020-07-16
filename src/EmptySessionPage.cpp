//
// Created by Fanchao Liu on 28/06/20.
//

#include "EmptySessionPage.h"
#include "ui_EmptySessionPage.h"

#include "ClubRepository.h"
#include "NewSessionDialog.h"
#include "EditMemberDialog.h"
#include "FakeNames.h"

#include <QRandomGenerator>
#include <QMessageBox>

struct EmptySessionPage::Impl {
    ClubRepository *const repo;
    Ui::EmptySessionPage ui;
};

EmptySessionPage::EmptySessionPage(ClubRepository *repo, QWidget *parent)
        : QFrame(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    d->ui.createFakeButton->setVisible(false);
    d->ui.checkInRandomButton->setVisible(false);
#ifndef NDEBUG
    d->ui.createFakeButton->setVisible(true);
    d->ui.checkInRandomButton->setVisible(true);
#endif

    reload();
    connect(d->repo, &ClubRepository::clubInfoChanged, this, &EmptySessionPage::reload);
    connect(d->repo, &ClubRepository::sessionChanged, this, &EmptySessionPage::reload);
    connect(d->ui.closeButton, &QPushButton::clicked, this, &EmptySessionPage::clubClosed);
}

EmptySessionPage::~EmptySessionPage() {
    delete d;
}

void EmptySessionPage::reload() {
    d->ui.clubNameLabel->setText(tr("Welcome to %1").arg(d->repo->getClubInfo().name));
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
    connect(dialog, &EditMemberDialog::newMemberCreated, [=] {
        QMessageBox::information(this, tr("Welcome"),
                                 tr("Register successfully. Welcome to %1.").arg(d->repo->getClubInfo().name));
    });
}


void EmptySessionPage::on_createFakeButton_clicked() {
    for (const auto &name : FakeNames::names()) {
        const auto components = name.split(QStringLiteral(" "));
        d->repo->createMember(components[0], components[1],
                              QRandomGenerator::global()->generate() % 4 == 0 ? Member::Female : Member::Male,
                              QRandomGenerator::global()->bounded(levelMin, levelMax));
    }
}

void EmptySessionPage::on_checkInRandomButton_clicked() {
    if (auto lastSessionId = d->repo->getLastSession()) {
        auto members = d->repo->getMembers(NonCheckedIn{*lastSessionId});
        auto size = 50 - d->repo->getMembers(CheckedIn{*lastSessionId}).size();
        std::shuffle(members.begin(), members.end(), std::default_random_engine());
        for (int i = 0; i < size; i++) {
            d->repo->checkIn(members[i].id, *lastSessionId, i % 5 != 0);
        }
    }
}
