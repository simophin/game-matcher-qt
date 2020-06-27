//
// Created by Fanchao Liu on 27/06/20.
//

#include "WelcomePage.h"
#include "ui_WelcomePage.h"
#include "NewClubDialog.h"

#include <QFileDialog>

WelcomePage::WelcomePage(QWidget *parent)
        : QFrame(parent), ui(new Ui::WelcomePage()) {
    ui->setupUi(this);
}

void WelcomePage::on_openButton_clicked() {
    auto path = QFileDialog::getOpenFileName(this);
    if (path.isEmpty()) {
        return;
    }
    
    emit clubOpened(path);
}

void WelcomePage::on_createButton_clicked() {
    auto dialog = new NewClubDialog(this);
    connect(dialog, &NewClubDialog::clubCreated, [=](QString path) {
        emit clubOpened(path);
    });

    connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

    dialog->show();
}

WelcomePage::~WelcomePage() = default;
