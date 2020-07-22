//
// Created by Fanchao Liu on 22/07/20.
//

#include "ClubWindow.h"
#include "ui_ClubWindow.h"

#include "ClubRepository.h"

#include <QEvent>

struct ClubWindow::Impl {
    ClubRepository *const repo;
    Ui::ClubWindow ui;
};

ClubWindow::ClubWindow(ClubRepository *repo, QWidget *parent)
        : QMainWindow(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    repo->setParent(this);
}

ClubWindow::~ClubWindow() {
    delete d;
}

void ClubWindow::changeEvent(QEvent *evt) {
    QMainWindow::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

ClubWindow *ClubWindow::create(const QString &dbPath, QWidget *parent) {
    if (auto clubRepo = ClubRepository::open(nullptr, dbPath)) {
        return new ClubWindow(clubRepo, parent);
    }

    return nullptr;
}
