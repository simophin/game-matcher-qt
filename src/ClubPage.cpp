//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubPage.h"
#include "ui_ClubPage.h"

#include "clubrepository.h"
#include "SessionPage.h"
#include "EmptySessionPage.h"

#include <QErrorMessage>

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

//    if (createDialog) {
//        auto dialog = new NewSessionDialog(&d->repo, this);
//        connect(dialog, &NewSessionDialog::sessionCreated, this, &ClubPage::onSessionCreated);
//        connect(dialog, &NewSessionDialog::finished, dialog, &QObject::deleteLater);
//        dialog->setWindowTitle(tr("Start a new session"));
//        dialog->show();
//    } else {
//        onSessionCreated();
//    }
}

void ClubPage::onSessionCreated() {
    while (!d->ui.layout->isEmpty()) {
        d->ui.layout->removeItem(d->ui.layout->itemAt(0));
    }

//    if (auto session = d->repo.getLastSession()) {
//        d->ui.layout->addWidget(new SessionPage(&d->repo, *session, this), 1);
//    }
}

void ClubPage::openLastSession() {
    if (auto lastSession = d->repo.getLastSession()) {
        openSession(lastSession->session.id);
    }
}

void ClubPage::openSession(SessionId) {

}

ClubPage::~ClubPage() {
    delete d;
}