//
// Created by Fanchao Liu on 27/06/20.
//

#include "ClubHomePage.h"
#include "ui_ClubHomePage.h"

ClubHomePage::ClubHomePage(QWidget *parent)
        : QFrame(parent), ui(new Ui::ClubHomePage()) {
    ui->setupUi(this);
}

ClubHomePage::~ClubHomePage() = default;