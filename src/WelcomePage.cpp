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

    connect(ui->createButton, &QPushButton::clicked, [this] {
        auto dialog = new NewClubDialog(this);
        connect(dialog, &NewClubDialog::clubCreated, [=](QString path) {
            emit clubOpened(path);
        });

        connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

        dialog->show();
    });

    connect(ui->openButton, &QPushButton::clicked, [this] {
        auto path = QFileDialog::getOpenFileName(this);
        if (path.isEmpty()) {
            return;
        }

        emit clubOpened(path);
    });
}

void WelcomePage::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

WelcomePage::~WelcomePage() = default;
