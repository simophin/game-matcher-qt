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
    QSettings().setValue(SETTINGS_KEY_LAST_OPEN_CLUB_FILE, path);
    setCentralWidget(new ClubPage(path, this));
}

void MainWindow::reload()
{
    auto lastOpen = QSettings().value(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
    if (lastOpen.isNull() || !QFile(lastOpen.toString()).exists()) {
        auto page = new WelcomePage(this);
        connect(page, &WelcomePage::clubOpened, this, &MainWindow::onClubOpened);
        setCentralWidget(page);
    } else {
        auto page = new ClubPage(lastOpen.toString(), this);
        connect(page, &ClubPage::clubClosed, [=] {
            QSettings().remove(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
            reload();
        });
        setCentralWidget(page);
    }
}

MainWindow::~MainWindow() = default;
