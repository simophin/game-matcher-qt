//
// Created by Fanchao Liu on 26/06/20.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "WelcomePage.h"
#include "ClubPage.h"

#include <QSettings>
#include <QFile>
#include <QtDebug>

static auto SETTINGS_KEY_LAST_OPEN_CLUB_FILE = QStringLiteral("last_open_file");

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    reload();
}

void MainWindow::onClubOpened(QString path) {
    if (auto clubPage = ClubPage::create(path, this)) {
        QSettings().setValue(SETTINGS_KEY_LAST_OPEN_CLUB_FILE, path);
        setCentralWidget(clubPage);
    }
}

void MainWindow::reload()
{
    auto lastOpen = QSettings().value(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
    ClubPage *clubPage;
    if (lastOpen.isNull() || !QFile(lastOpen.toString()).exists() || !(clubPage = ClubPage::create(lastOpen.toString(), this))) {
        auto page = new WelcomePage(this);
        connect(page, &WelcomePage::clubOpened, this, &MainWindow::onClubOpened);
        setCentralWidget(page);
    } else {
        connect(clubPage, &ClubPage::clubClosed, [=] {
            QSettings().remove(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
            reload();
        });
        setCentralWidget(clubPage);
    }
}

MainWindow::~MainWindow() = default;
