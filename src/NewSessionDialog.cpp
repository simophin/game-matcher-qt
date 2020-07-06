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

static const SettingKey skLastNumCourts = QStringLiteral("last_num_courts");
static const SettingKey skLastNumPlayersPerCourt = QStringLiteral("last_num_player_per_court");
static const SettingKey skLastSessionFee = QStringLiteral("last_session_fee");
static const SettingKey skLastPlace = QStringLiteral("last_place");
static const SettingKey skLastAnnouncement = QStringLiteral("last_announcement");

NewSessionDialog::NewSessionDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    auto validator = new QDoubleValidator(this);
    validator->setDecimals(2);
    validator->setNotation(QDoubleValidator::StandardNotation);
    d->ui.feeLineEdit->setValidator(validator);
    bool ok;
    if (int feeInCents = repo->getSetting(skLastSessionFee).toInt(&ok); ok) {
        d->ui.feeLineEdit->setText(QString::number(feeInCents / 100.0));
    }
    d->ui.numberOfCourtsSpinBox->setValue(repo->getSetting(skLastNumCourts).toInt());
    d->ui.placeValue->setText(repo->getSetting(skLastPlace));
    d->ui.annoucement->setText(repo->getSetting(skLastAnnouncement));
    d->ui.numberOfPlayersPerCourtSpinBox->setValue(repo->getSetting(skLastNumPlayersPerCourt).toInt());

    validateForm();
    connect(d->ui.feeLineEdit, &QLineEdit::textChanged, this, &NewSessionDialog::validateForm);
    connect(d->ui.placeValue, &QLineEdit::textChanged, this, &NewSessionDialog::validateForm);
    connect(d->ui.numberOfCourtsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &NewSessionDialog::validateForm);
    connect(d->ui.numberOfPlayersPerCourtSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &NewSessionDialog::validateForm);
}

void NewSessionDialog::validateForm() {
    if (auto btn = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        btn->setEnabled(
                d->ui.feeLineEdit->hasAcceptableInput() &&
                !d->ui.placeValue->text().trimmed().isEmpty() &&
                d->ui.numberOfCourtsSpinBox->value() > 0 &&
                d->ui.numberOfPlayersPerCourtSpinBox->value() > 0);
    }
}

void NewSessionDialog::accept() {
    QVector<CourtConfiguration> courts;
    auto numCourts = d->ui.numberOfCourtsSpinBox->value();
    auto numPlayersPerCourt = d->ui.numberOfPlayersPerCourtSpinBox->value();
    for (auto i = 0, size = numCourts; i < size; i++) {
        courts.append(CourtConfiguration{QString::number(i + 1), -i});
    }

    int fee = d->ui.feeLineEdit->text().toDouble() * 100;
    auto announcement = d->ui.annoucement->toPlainText().trimmed();
    auto place = d->ui.placeValue->text().trimmed();
    if (d->repo->createSession(
            fee,
            place,
            announcement,
            numPlayersPerCourt,
            courts)) {

        d->repo->saveSettings(skLastNumCourts, numCourts);
        d->repo->saveSettings(skLastSessionFee, fee);
        d->repo->saveSettings(skLastAnnouncement, announcement);
        d->repo->saveSettings(skLastPlace, place);
        d->repo->saveSettings(skLastNumPlayersPerCourt, numPlayersPerCourt);

        emit this->sessionCreated();
        QDialog::accept();
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