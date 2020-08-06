//
// Created by Fanchao Liu on 23/07/20.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ClubPage.h"
#include "WelcomePage.h"
#include "ToastDialog.h"

#include <QEvent>
#include <QSettings>
#include <QFile>
#include <QMessageBox>

static const auto skLastOpened = QStringLiteral("last_opened");
static auto skMainWindowGeometry = QStringLiteral("sk_main_window_geometry");

struct MainWindow::Impl {
    Ui::MainWindow ui;
};

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), d(new Impl) {
    d->ui.setupUi(this);
    ToastDialog::registerMainWindow(this);

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
        connect(clubPage, &ClubPage::toggleFullScreenRequested, [=] {
            if (isFullScreen()) {
                showNormal();
            } else {
                showFullScreen();
            }
        });
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

void MainWindow::closeEvent(QCloseEvent *event) {
    QSettings().setValue(skMainWindowGeometry, saveGeometry());
    QWidget::closeEvent(event);
}

bool MainWindow::restoreFromSettings(const QSettings &settings) {
    auto value = settings.value(skMainWindowGeometry);
    if (value.isNull()) return false;
    return restoreGeometry(value.toByteArray());
}
