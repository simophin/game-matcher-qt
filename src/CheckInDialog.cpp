//
// Created by Fanchao Liu on 30/06/20.
//
#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

#include "ClubRepository.h"
#include "ToastDialog.h"

#include <QMessageBox>
#include <QLocale>
#include <QPushButton>
#include <set>


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
    d->ui.annoucement->setText(tr("<u>Club announcement</u><br />%1").arg(d->session.session.announcement));
    d->ui.annoucement->setVisible(!d->session.session.announcement.isEmpty());

    if (sessionFee == 0) {
        d->ui.paidButton->setText(tr("OK"));
        d->ui.unpaidButton->hide();
    }

    connect(d->ui.paidButton, &QPushButton::clicked, [=] {
        doCheckIn(true);
    });

    connect(d->ui.unpaidButton, &QPushButton::clicked, [=] {
        doCheckIn(false);
    });

    connect(d->ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

CheckInDialog::~CheckInDialog() {
    delete d;
}

void CheckInDialog::doCheckIn(bool paid) {
    if (!d->repo->checkIn(d->session.session.id, d->id, paid)) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to check in. \nYou probably have already checked in. Check the board!"));
        return;
    }

    if (auto member = d->repo->getMember(d->id)) {
        ToastDialog::show(tr("%1 checked in successfully").arg(member->fullName()));
    }

    emit this->memberCheckedIn(d->id);
    accept();
}