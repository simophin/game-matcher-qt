//
// Created by Fanchao Liu on 30/06/20.
//

#include "EditMemberDialog.h"
#include "ui_EditMemberDialog.h"

#include "ClubRepository.h"

#include <optional>
#include <QMessageBox>

struct EditMemberDialog::Impl {
    ClubRepository *repo;
    std::optional<MemberId> editingMember;
    Ui::EditMemberDialog ui;
};

EditMemberDialog::EditMemberDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    d->ui.genderComboBox->addItem(tr("Boy", "gender"), genderMale);
    d->ui.genderComboBox->addItem(tr("Girl", "gender"), genderFemale);

    for (int i = levelMin; i <= levelMax; i++) {
        QString levelDesc;
        if (i == levelMin) {
            levelDesc = tr("Newbie", "level");
        } else if (i == (levelMax - levelMin) / 2) {
            levelDesc = tr("Medium", "level");
        } else if (i == levelMax) {
            levelDesc = tr("Expert", "level");
        }

        if (levelDesc.isNull()) {
            d->ui.levelComboBox->addItem(QString::number(i), i);
        } else {
            d->ui.levelComboBox->addItem(tr("%1 - %2", "level with desc")
                                                 .arg(QString::number(i), levelDesc), i);
        }
    }
}

EditMemberDialog::~EditMemberDialog() {
    delete d;
}

void EditMemberDialog::setMember(MemberId id) {
    if (auto m = d->repo->getMember(id)) {
        d->editingMember = m->id;
        d->ui.firstNameLineEdit->setText(m->firstName);
        d->ui.lastNameLineEdit->setText(m->lastName);
        if (auto index = d->ui.levelComboBox->findData(m->level); index >= 0) {
            d->ui.levelComboBox->setCurrentIndex(index);
        }
        if (auto index = d->ui.genderComboBox->findData(m->gender); index >= 0) {
            d->ui.genderComboBox->setCurrentIndex(index);
        }
        d->ui.emailLineEdit->setText(m->email);
        d->ui.phoneLineEdit->setText(m->phone);
    }
}

void EditMemberDialog::accept() {
    QStringList errors;

    auto firstName = d->ui.firstNameLineEdit->text().trimmed();
    auto lastName = d->ui.lastNameLineEdit->text().trimmed();

    if (!firstName.isEmpty() && !lastName.isEmpty() && d->repo->hasMember(firstName, lastName)) {
        errors.append(
                tr("* Your name \"%1 %2\" is taken. You can try adding a middle name to the last name").arg(firstName,
                                                                                                          lastName));
    }

    if (firstName.isEmpty()) {
        errors.append(tr("* You must fill in your first name"));
    }

    if (lastName.isEmpty()) {
        errors.append(tr("* You must fill in your last name"));
    }

    auto level = d->ui.levelComboBox->currentData();
    if (level.isNull()) {
        errors.append((tr("* You must select your playing level")));
    }

    auto gender = d->ui.genderComboBox->currentData();
    if (gender.isNull()) {
        errors.append(tr("* You must select your gender"));
    }

    if (!errors.isEmpty()) {
        QMessageBox::critical(this, tr("Can not save your form"), errors.join(QStringLiteral("<br />")));
        return;
    }

    if (d->editingMember) {
        Member m;
        m.id = *d->editingMember;
        m.firstName = firstName;
        m.lastName = lastName;
        m.phone = d->ui.phoneLineEdit->text();
        m.email = d->ui.emailLineEdit->text();
        m.level = level.toInt();
        m.gender = gender.toString();
        if (!d->repo->saveMember(m)) {
            QMessageBox::warning(this, tr("Unable to save to database"),
                    tr("Probably nothing you can do about this. You can ignore this."));
        } else {
            QMessageBox::information(this, tr("Success"),tr("Update successfully"));
            QDialog::accept();
        }
    } else if (auto newMember = d->repo->createMember(firstName, lastName, gender.toString(), level.toInt())) {
        emit this->newMemberCreated(newMember->id);
        QMessageBox::information(this, tr("Welcome"),
                tr("Register successfully. Welcome to %1.").arg(d->repo->clubInfo().name));
        QDialog::accept();
    } else {
        QMessageBox::warning(this, tr("Unable to save to database"),
                             tr("You can try to save the form again or change something in the form"));
    }
}
