//
// Created by Fanchao Liu on 30/06/20.
//
#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

#include "ClubRepository.h"

#include <QMessageBox>
#include <QLocale>
#include <QPushButton>
#include <set>

#include "ToastEvent.h"

struct CheckInDialog::Impl {
    MemberId id;
    ClubRepository *repo;
    SessionData session;
    Ui::CheckInDialog ui;
};

CheckInDialog::CheckInDialog(MemberId id, SessionId sessionId, ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{id, repo}) {
    d->ui.setupUi(this);

    if (auto session = repo->getSession(sessionId); session) {
        d->session = *session;
    } else {
        QMessageBox::critical(this, tr("Unable to find current session"), tr("Please try again"));
        close();
        return;
    }

    auto member = repo->getMember(id);
    if (!member) {
        QMessageBox::critical(this, tr("Unable to find this member"), tr("Please try again"));
        close();
        return;
    }

    d->ui.nameValueLabel->setText(member->fullName());
    auto sessionFee = d->session.session.fee;
    d->ui.feeValueLabel->setText(QLocale::c().toCurrencyString(sessionFee / 100.0));
    d->ui.annoucement->setText(d->session.session.announcement);
    d->ui.annoucement->setVisible(!d->session.session.announcement.isEmpty());
    connect(d->ui.paidRadioButton, &QRadioButton::toggled, this, &CheckInDialog::validateForm);
    connect(d->ui.unpaidRadioButton, &QRadioButton::toggled, this, &CheckInDialog::validateForm);
    validateForm();
}

CheckInDialog::~CheckInDialog() {
    delete d;
}

void CheckInDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        button->setEnabled(d->ui.paidRadioButton->isChecked() || d->ui.unpaidRadioButton->isChecked());
    }
}

void CheckInDialog::accept() {
    if (!d->repo->checkIn(d->id, d->session.session.id, d->ui.paidRadioButton->isChecked())) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to check in. \nYou probably have already checked in. Check the board!"));
        return;
    }

    if (auto member = d->repo->getMember(d->id)) {
        ToastEvent::show(tr("%1 checked in successfully").arg(member->fullName()));
    }

    emit this->memberCheckedIn(d->id);
    QDialog::accept();
}
