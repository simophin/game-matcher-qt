//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubPage.h"
#include "ui_ClubPage.h"

#include "ClubRepository.h"
#include "SessionPage.h"
#include "EmptySessionPage.h"

#include <QDialog>
#include <QMessageBox>
#include <QStackedLayout>

struct ClubPage::Impl {
    ClubRepository * const repo;
    QStackedLayout *layout;
    Ui::ClubPage ui;
};

ClubPage::ClubPage(ClubRepository *repo, QWidget *parent)
        : QFrame(parent), d(new Impl{repo}) {
    d->repo->setParent(this);
    d->ui.setupUi(this);
    setLayout(d->layout = new QStackedLayout());
    d->layout->setStackingMode(QStackedLayout::StackOne);

    auto page = new EmptySessionPage(d->repo, this);
    connect(page, &EmptySessionPage::lastSessionResumed, this, &ClubPage::openLastSession);
    connect(page, &EmptySessionPage::newSessionCreated, this, &ClubPage::openLastSession);
    connect(page, &EmptySessionPage::clubClosed, this, &ClubPage::clubClosed);
    d->layout->addWidget(page);

    setWindowTitle(repo->getClubName());
}


void ClubPage::openLastSession() {
    if (auto lastSession = d->repo->getLastSession()) {
        openSession(*lastSession);
    }
}

void ClubPage::openSession(SessionId sessionId) {
    if (auto session = SessionPage::create(sessionId, d->repo, this)) {
        d->layout->addWidget(session);
        d->layout->setCurrentWidget(session);
        connect(session, &SessionPage::closeSessionRequested, [=] {
            d->layout->removeWidget(session);
            session->deleteLater();
        });

        connect(session, &SessionPage::toggleFullScreenRequested, this, &ClubPage::toggleFullScreenRequested);
    } else {
        QMessageBox::warning(this, tr("Error opening session"), tr("Unable to open session page"));
    }
}

ClubPage::~ClubPage() {
    delete d;
}

ClubPage *ClubPage::create(const QString &dbPath, QWidget *parent) {
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, dbPath));
    if (!repo) {
        QMessageBox::warning(parent, tr("Error"), tr("Unable to open \"%1\"").arg(dbPath));
        return nullptr;
    }

    return new ClubPage(repo.release(), parent);
}

ClubRepository *ClubPage::clubRepository() const {
    return d->repo;
}
