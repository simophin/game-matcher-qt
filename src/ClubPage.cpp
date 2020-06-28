//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubPage.h"
#include "ui_ClubPage.h"

#include "clubrepository.h"
#include "SessionPage.h"
#include "NewSessionDialog.h"
#include "EmptySessionPage.h"

#include <QErrorMessage>
#include <QMessageBox>

struct ClubPage::Impl {
    ClubRepository repo;
};

ClubPage::ClubPage(const QString &path, QWidget *parent)
        : QFrame(parent), d(new Impl), ui(new Ui::ClubPage()) {
    ui->setupUi(this);
    if (!d->repo.open(path)) {
        (new QErrorMessage(this))->showMessage(tr("Unable to open club file \"%1\"").arg(path));
        deleteLater();
    }

    bool createDialog;
    auto session = d->repo.getLastSession();
    if (session && qAbs(session->session.startTime.daysTo(QDateTime::currentDateTime())) > 0) {
        createDialog = QMessageBox::question(this, tr("Question about last session"),
                                             tr("Last session is long time ago. Do you want to start a new one?")) == QMessageBox::Yes;
    } else if (!session) {
        createDialog = true;
    } else {
        createDialog = false;
    }

    ui->layout->addWidget(new EmptySessionPage(&d->repo, this));

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
    while (!ui->layout->isEmpty()) {
        ui->layout->removeItem(ui->layout->itemAt(0));
    }

//    if (auto session = d->repo.getLastSession()) {
//        ui->layout->addWidget(new SessionPage(&d->repo, *session, this), 1);
//    }
}

ClubPage::~ClubPage() = default;