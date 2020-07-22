//
// Created by Fanchao Liu on 26/06/20.
//

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "WelcomePage.h"
#include "ClubPage.h"
#include "ToastDialog.h"
#include "ToastEvent.h"
#include "ClubWindow.h"

#include <QSettings>
#include <QFile>

static auto SETTINGS_KEY_LAST_OPEN_CLUB_FILE = QStringLiteral("last_open_file");

struct MainWindow::Impl {
    ToastDialog *toastDialog;
    Ui::MainWindow ui;
};

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), d(new Impl{new ToastDialog(this)}) {
    d->ui.setupUi(this);

    reload();
    QCoreApplication::instance()->installEventFilter(this);
}

void MainWindow::onClubOpened(QString path) {
    if (auto clubWindow = ClubWindow::create(path, this)) {
        QSettings().setValue(SETTINGS_KEY_LAST_OPEN_CLUB_FILE, path);
        clubWindow->showMaximized();
        hide();
        connect(clubWindow, &QMainWindow::destroyed, this, &QMainWindow::show);
    }
}

void MainWindow::reload()
{
    auto lastOpen = QSettings().value(SETTINGS_KEY_LAST_OPEN_CLUB_FILE);
    if (lastOpen.isNull() || !QFile(lastOpen.toString()).exists()) {
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

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (auto toast = dynamic_cast<ToastEvent*>(event)) {
        d->toastDialog->showMessage(toast->msg(), toast->delayMills());
        return true;
    }

    return QObject::eventFilter(watched, event);
}

MainWindow::~MainWindow() {
    delete d;
}
