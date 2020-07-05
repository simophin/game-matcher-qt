//
// Created by Fanchao Liu on 30/06/20.
//

#include "MemberSelectDialog.h"
#include "ui_MemberSelectDialog.h"

#include "ClubRepository.h"

#include <QPushButton>
#include <QTimer>

struct MemberSelectDialog::Impl {
    MemberSearchFilter filter;
    ClubRepository *repo;
    QTimer *filterDebounceTimer;
    Ui::MemberSelectDialog ui;
};

MemberSelectDialog::MemberSelectDialog(MemberSearchFilter filter, ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{filter, repo, new QTimer(this)}) {
    d->ui.setupUi(this);
    d->filterDebounceTimer->setSingleShot(true);
    d->filterDebounceTimer->setInterval(500);

    applyData();
    connect(d->filterDebounceTimer, &QTimer::timeout, this, &MemberSelectDialog::applyData);
    connect(d->ui.filterEdit, &QLineEdit::textChanged, d->filterDebounceTimer, qOverload<>(&QTimer::start));

    connect(d->ui.memberList, &QListWidget::currentItemChanged, this, &MemberSelectDialog::validateForm);
    validateForm();
}

MemberSelectDialog::~MemberSelectDialog() {
    delete d;
}

void MemberSelectDialog::applyData() {
    QVector<Member> members;

    if (auto needle = d->ui.filterEdit->text().trimmed(); !needle.isEmpty()) {
        members = d->repo->findMember(d->filter, needle);
    } else {
        members = d->repo->getMembers(d->filter);
    }

    d->ui.memberList->clear();
    QFont font;
    font.setPointSize(16);
    for (const auto &member : members) {
        auto item = new QListWidgetItem(member.fullName(), d->ui.memberList);
        item->setFont(font);
        item->setData(Qt::UserRole, member.id);
    }
}

void MemberSelectDialog::on_memberList_itemDoubleClicked(QListWidgetItem *item) {
    if (auto data = item->data(Qt::UserRole); data.isValid()) {
        emit this->memberSelected(data.toLongLong());
        close();
    }
}

void MemberSelectDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        button->setEnabled(d->ui.memberList->currentItem() != nullptr);
    }
}

void MemberSelectDialog::accept() {
    auto currentItem = d->ui.memberList->currentItem();
    if (!currentItem) {
        return;
    }

    if (auto data = currentItem->data(Qt::UserRole); data.isValid()) {
        emit this->memberSelected(data.toLongLong());
        QDialog::accept();
    }
}

void MemberSelectDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}
