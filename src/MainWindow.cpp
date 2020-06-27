//
// Created by Fanchao Liu on 26/06/20.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "WelcomePage.h"
#include "ClubHomePage.h"

#include <QSettings>
#include <QFile>
#include <QtDebug>

static auto SETTINGS_KEY_LAST_OPEN_CLUB_FILE = QStringLiteral("last_open_file");

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    auto lastOpen = QSettings().value(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
    if (lastOpen.isNull() || !QFile(lastOpen.toString()).exists()) {
        auto page = new WelcomePage(this);
        connect(page, &WelcomePage::clubOpened, this, &MainWindow::onClubOpened);
        setCentralWidget(page);
    } else {
        setCentralWidget(new ClubHomePage(this));
    }
}

void MainWindow::onClubOpened(QString path) {
    QSettings().setValue(SETTINGS_KEY_LAST_OPEN_CLUB_FILE, path);
    setCentralWidget(new ClubHomePage(this));
}

MainWindow::~MainWindow() = default;