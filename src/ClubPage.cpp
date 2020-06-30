//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubPage.h"
#include "ui_ClubPage.h"

#include "ClubRepository.h"
#include "SessionDialog.h"
#include "EmptySessionPage.h"

#include <QErrorMessage>
#include <QDialog>

struct ClubPage::Impl {
    ClubRepository repo;
    Ui::ClubPage ui;
};

ClubPage::ClubPage(const QString &path, QWidget *parent)
        : QFrame(parent), d(new Impl) {
    d->ui.setupUi(this);
    if (!d->repo.open(path)) {
        (new QErrorMessage(this))->showMessage(tr("Unable to open club file \"%1\"").arg(path));
        deleteLater();
    }

    auto page = new EmptySessionPage(&d->repo, this);
    connect(page, &EmptySessionPage::lastSessionResumed, this, &ClubPage::openLastSession);
    connect(page, &EmptySessionPage::newSessionCreated, this, &ClubPage::openLastSession);
    d->ui.layout->addWidget(page);
}


void ClubPage::openLastSession() {
    if (auto lastSession = d->repo.getLastSession()) {
        openSession(*lastSession);
    }
}

void ClubPage::openSession(SessionId sessionId) {
    auto session = new SessionDialog(&d->repo, sessionId, this);
    session->show();
}

ClubPage::~ClubPage() {
    delete d;
}