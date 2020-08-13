//
// Created by Fanchao Liu on 30/06/20.
//

#include "EditMemberDialog.h"
#include "ui_EditMemberDialog.h"

#include "ClubRepository.h"
#include "TypeUtils.h"

#include <optional>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "ToastDialog.h"

struct EditMemberDialog::Impl {
    ClubRepository *repo;
    std::optional<MemberId> editingMember;
    Ui::EditMemberDialog ui;
    QTimer showNameErrorTimer = QTimer();

    static std::optional<std::pair<QString, QString>> splitNames(const QString &fullName) {
        auto firstSpaceIndex = fullName.indexOf(QStringLiteral(" "));
        if (firstSpaceIndex < 0 || firstSpaceIndex == fullName.size() - 1) return std::nullopt;
        QString lastName = fullName.mid(firstSpaceIndex + 1).trimmed();
        if (lastName.isEmpty()) return std::nullopt;

        return std::make_pair(fullName.left(firstSpaceIndex), lastName);
    }
};

EditMemberDialog::EditMemberDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    d->ui.genderComboBox->addItem(tr("M", "gender"), Member::Male);
    d->ui.genderComboBox->addItem(tr("F", "gender"), Member::Female);

    d->showNameErrorTimer.setSingleShot(false);
    d->showNameErrorTimer.setInterval(800);
    connect(&d->showNameErrorTimer, &QTimer::timeout, d->ui.fullNameErrorLabel, &QWidget::show);
    d->ui.fullNameErrorLabel->hide();

    connect(d->ui.fullNameValue, &QLineEdit::textChanged, this, &EditMemberDialog::validateForm);
    connect(d->ui.genderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &EditMemberDialog::validateForm);
    connect(d->ui.levelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &EditMemberDialog::validateForm);

    auto[levelMin, levelMax] = repo->getLevelRange();

    for (int i = levelMin; i <= levelMax; i++) {
        QString levelDesc;
        if (i == levelMin) {
            levelDesc = tr("Experienced", "level");
        } else if (i == levelMax) {
            levelDesc = tr("Beginner", "level");
        }

        if (levelDesc.isNull()) {
            d->ui.levelComboBox->addItem(QString::number(i), i);
        } else {
            d->ui.levelComboBox->addItem(tr("%1 - %2", "level with desc")
                                                 .arg(QString::number(i), levelDesc), i);
        }
    }

    validateForm();
}

EditMemberDialog::~EditMemberDialog() {
    delete d;
}

void EditMemberDialog::setMember(MemberId id) {
    if (auto m = d->repo->getMember(id)) {
        d->editingMember = m->id;
        d->ui.fullNameValue->setText(m->fullName());
        if (auto index = d->ui.levelComboBox->findData(m->level); index >= 0) {
            d->ui.levelComboBox->setCurrentIndex(index);
        }
        if (auto index = d->ui.genderComboBox->findData(m->gender); index >= 0) {
            d->ui.genderComboBox->setCurrentIndex(index);
        }

        d->ui.phoneLineEdit->setText(m->phone);
        d->ui.emailLineEdit->setText(m->email);

        setWindowTitle(tr("Editing member info"));
    }
}

void EditMemberDialog::accept() {
    auto phone = d->ui.phoneLineEdit->text().trimmed();
    auto email = d->ui.emailLineEdit->text().trimmed();
    if (phone.isEmpty() && email.isEmpty()) {
        QMessageBox::critical(
                this, tr("Can not save your form"),
                tr("You need to at least give us a phone number or the email address")
        );
        return;
    }

    auto names = d->splitNames(d->ui.fullNameValue->text());
    if (!names) return;

    auto[firstName, lastName] = *names;

    auto existingMember = d->repo->findMemberBy(firstName, lastName);
    if ((d->editingMember && existingMember && d->editingMember != existingMember) ||
        (!d->editingMember && existingMember)) {
        QMessageBox::critical(
                this, tr("Can not save your form"),
                tr("The name '%1 %2' is already taken. \nIf you have already registered, you just need to check in. \nOtherwise, you can put in your middle names.").arg(
                        firstName, lastName)
        );
        return;
    }

    auto level = d->ui.levelComboBox->currentData().toInt();;
    auto gender = d->ui.genderComboBox->currentData().value<Member::Gender>();

    if (d->editingMember) {
        Member m;
        m.id = *d->editingMember;
        m.firstName = firstName;
        m.lastName = lastName;
        m.level = level;
        m.gender = gender;
        if (!d->repo->saveMember(m)) {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Unable to save to database. Probably nothing you can do about this. You should try again."));
        } else {
            emit this->memberUpdated(*d->editingMember);
            ToastDialog::show(tr("Member %1 updated successfully").arg(m.fullName()));
            QDialog::accept();
        }
    } else if (auto newMember = d->repo->createMember(firstName, lastName, gender, level, phone, email)) {
        emit this->newMemberCreated(newMember->id);
        ToastDialog::show(tr("Member %1 created successfully").arg(newMember->fullName()));
        QDialog::accept();
    } else {
        QMessageBox::warning(this, tr("Unable to save to database"),
                             tr("You can try to save the form again or change something in the form"));
    }
}

void EditMemberDialog::validateForm() {
    auto fullName = d->ui.fullNameValue->text();
    bool isValid = true;
    if (!fullName.isEmpty() && !d->splitNames(fullName)) {
        d->showNameErrorTimer.start();
        isValid = false;
    } else {
        d->showNameErrorTimer.stop();
        d->ui.fullNameErrorLabel->hide();
        isValid &= !fullName.isEmpty();
    }

    isValid &= d->ui.levelComboBox->currentData().isValid();
    isValid &= d->ui.genderComboBox->currentData().isValid();

    d->ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(isValid);
}
