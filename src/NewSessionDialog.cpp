//
// Created by Fanchao Liu on 27/06/20.
//

#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "clubrepository.h"

struct NewSessionDialog::Impl {
    ClubRepository *repo;
    Ui::NewSessionDialog ui;
};

NewSessionDialog::NewSessionDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
}

NewSessionDialog::~NewSessionDialog() = default;