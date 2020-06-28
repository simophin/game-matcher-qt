//
// Created by Fanchao Liu on 27/06/20.
//
#include "SessionPage.h"
#include "ui_SessionPage.h"

#include "clubrepository.h"

struct SessionPage::Impl {
    ClubRepository *repo;
    SessionData session;
    Ui::SessionPage ui;
};

SessionPage::SessionPage(ClubRepository *repo, const SessionData &s, QWidget *parent)
        : QFrame(parent), d(new Impl{repo, s}) {
    d->ui.setupUi(this);
}

SessionPage::~SessionPage() = default;