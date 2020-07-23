//
// Created by Fanchao Liu on 23/07/20.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ClubPage.h"
#include "WelcomePage.h"

#include <QEvent>
#include <QSettings>
#include <QFile>

static const auto skLastOpened = QStringLiteral("last_opened");

struct MainWindow::Impl {
    Ui::MainWindow ui;
};

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), d(new Impl) {
    d->ui.setupUi(this);

    auto lastOpened = QSettings().value(skLastOpened).toString();
    if (!lastOpened.isNull() && QFile(lastOpened).exists() && openClub(lastOpened)) return;

    openWelcomePage();
}

MainWindow::~MainWindow() {
    delete d;
}

bool MainWindow::openClub(const QString &dbPath) {
    if (auto clubPage = ClubPage::create(dbPath, nullptr)) {
        setCentralWidget(clubPage);
        connect(clubPage, &ClubPage::clubClosed, this, &MainWindow::openWelcomePage);
        QSettings().setValue(skLastOpened, dbPath);
        setWindowTitle(clubPage->windowTitle());
        return true;
    }

    return false;
}

void MainWindow::openWelcomePage() {
    auto page = new WelcomePage();
    setCentralWidget(page);
    connect(page, &WelcomePage::clubOpened, this, &MainWindow::openClub);
    setWindowTitle(page->windowTitle());
    QSettings().remove(skLastOpened);
}

void MainWindow::changeEvent(QEvent *evt) {
    QMainWindow::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}
