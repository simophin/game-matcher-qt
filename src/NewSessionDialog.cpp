//
// Created by Fanchao Liu on 27/06/20.
//

#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "ClubRepository.h"

#include <QPushButton>
#include <QMessageBox>

struct NewSessionDialog::Impl {
    ClubRepository *repo;
    Ui::NewSessionDialog ui;
};

static const auto settingsKeyLastNumCourts = QStringLiteral("last_num_courts");

NewSessionDialog::NewSessionDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    auto validator = new QDoubleValidator(this);
    validator->setDecimals(2);
    validator->setNotation(QDoubleValidator::StandardNotation);
    d->ui.feeLineEdit->setValidator(validator);
    d->ui.feeLineEdit->setText(QString::number(repo->clubInfo().sessionFee / 100.0));

    d->ui.numberOfCourtsSpinBox->setValue(repo->getSettings(settingsKeyLastNumCourts).toInt());
    connect(d->ui.numberOfCourtsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (value > 0) {
            d->repo->setSettings(settingsKeyLastNumCourts, value);
        }
    });

    validateForm();
    connect(d->ui.feeLineEdit, &QLineEdit::textChanged, this, &NewSessionDialog::validateForm);
    connect(d->ui.numberOfCourtsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &NewSessionDialog::validateForm);
}

void NewSessionDialog::validateForm() {
    if (auto btn = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        btn->setEnabled(
                d->ui.feeLineEdit->hasAcceptableInput() &&
                d->ui.numberOfCourtsSpinBox->value() > 0);
    }
}

void NewSessionDialog::accept() {
}

void NewSessionDialog::on_buttonBox_accepted() {
    QVector<CourtConfiguration> courts;
    for (auto i = 0, size = d->ui.numberOfCourtsSpinBox->value(); i < size; i++) {
        courts.append(CourtConfiguration { tr("Court %1").arg(i + 1), -i });
    }

    if (d->repo->createSession(
            d->ui.feeLineEdit->text().toDouble() * 100,
            d->ui.annoucement->toPlainText(),
            courts)) {
        emit this->sessionCreated();
        close();
    } else {
        QMessageBox::warning(this, tr("Unable to start new session"), tr("Please try again"));
    }

}

void NewSessionDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

NewSessionDialog::~NewSessionDialog() = default;