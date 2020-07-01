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
    ui->feePerSessionLineEdit->setValidator(validator);
    validateForm();


    connect(ui->clubNameLineEdit, &QLineEdit::textChanged, this, &NewClubDialog::validateForm);
    connect(ui->feePerSessionLineEdit, &QLineEdit::textChanged, this, &NewClubDialog::validateForm);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=] {
        auto path = QFileDialog::getSaveFileName(this);

        ClubRepository club;
        ClubInfo info;
        info.name = ui->clubNameLineEdit->text().trimmed();
        info.sessionFee = ui->feePerSessionLineEdit->text().toDouble() * 100;
        info.creationDate = QDateTime::currentDateTimeUtc();
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
        saveButton->setEnabled(!ui->clubNameLineEdit->text().trimmed().isEmpty() &&
            ui->feePerSessionLineEdit->hasAcceptableInput());
    }
}

void NewClubDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

NewClubDialog::~NewClubDialog() = default;