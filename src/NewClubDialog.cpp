//
// Created by Fanchao Liu on 26/06/20.
//

#include <QDoubleValidator>
#include <QPushButton>
#include <QFileDialog>
#include <QErrorMessage>

#include "NewClubDialog.h"
#include "ui_NewClubDialog.h"
#include "ClubRepository.h"

NewClubDialog::NewClubDialog(QWidget *parent)
        : QDialog(parent), ui(new Ui::NewClubDialog()) {
    ui->setupUi(this);

    auto validator = new QDoubleValidator(this);
    validator->setBottom(0.0);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setDecimals(2);
    validateForm();


    connect(ui->clubNameLineEdit, &QLineEdit::textChanged, this, &NewClubDialog::validateForm);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=] {
        auto path = QFileDialog::getSaveFileName(this);

        ClubRepository club;
        ClubInfo info = {
                ui->clubNameLineEdit->text().trimmed()
        };
        if (!club.open(path) || !club.saveClubInfo(info)) {
            (new QErrorMessage(this))->showMessage(tr("Unable to create \"%1\"").arg(path));
            return;
        }

        club.close();
        emit clubCreated(path);
        close();
    });
}

void NewClubDialog::accept() {}

void NewClubDialog::validateForm() {
    if (auto saveButton = ui->buttonBox->button(QDialogButtonBox::Save)) {
        saveButton->setEnabled(!ui->clubNameLineEdit->text().trimmed().isEmpty());
    }
}

void NewClubDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

NewClubDialog::~NewClubDialog() = default;