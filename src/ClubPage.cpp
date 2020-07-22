//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubPage.h"
#include "ui_ClubPage.h"

#include "ClubRepository.h"
#include "SessionWindow.h"
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
    d->layout->setStackingMode(QStackedLayout::StackAll);

    auto page = new EmptySessionPage(d->repo, this);
    connect(page, &EmptySessionPage::lastSessionResumed, this, &ClubPage::openLastSession);
    connect(page, &EmptySessionPage::newSessionCreated, this, &ClubPage::openLastSession);
    connect(page, &EmptySessionPage::clubClosed, this, &ClubPage::clubClosed);
    d->layout->addWidget(page);
}


void ClubPage::openLastSession() {
    if (auto lastSession = d->repo->getLastSession()) {
        openSession(*lastSession);
    }
}

void ClubPage::openSession(SessionId sessionId) {
    auto session = new SessionWindow(d->repo, sessionId, this);
    d->layout->addWidget(session);
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
