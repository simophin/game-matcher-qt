//
// Created by Fanchao Liu on 30/06/20.
//

#include "MemberSelectDialog.h"
#include "ui_MemberSelectDialog.h"

#include "ClubRepository.h"
#include "EditMemberDialog.h"
#include "MemberMenu.h"
#include "MemberPainter.h"

#include <QPushButton>
#include <QTimer>
#include <QListWidgetItem>

struct MemberSelectDialog::Impl {
    MemberSearchFilter filter;
    ClubRepository *repo;
    QTimer *filterDebounceTimer;
    bool closeWhenSelected;
    Ui::MemberSelectDialog ui;
};

MemberSelectDialog::MemberSelectDialog(MemberSearchFilter filter, bool showRegister, bool closeWhenSelected, ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{filter, repo, new QTimer(this), closeWhenSelected}) {
    d->ui.setupUi(this);
    d->filterDebounceTimer->setSingleShot(true);
    d->filterDebounceTimer->setInterval(500);

    d->ui.registerButton->setVisible(showRegister);

    connect(d->repo, &ClubRepository::sessionChanged, this, &MemberSelectDialog::reload, Qt::QueuedConnection);
    connect(d->repo, &ClubRepository::memberChanged, this, &MemberSelectDialog::reload, Qt::QueuedConnection);
    connect(d->filterDebounceTimer, &QTimer::timeout, this, &MemberSelectDialog::reload);
    connect(d->ui.filterEdit, &QLineEdit::textChanged, d->filterDebounceTimer, qOverload<>(&QTimer::start));

    connect(d->ui.memberList, &QListWidget::itemSelectionChanged, this, &MemberSelectDialog::validateForm);
    connect(d->ui.memberList, &QWidget::customContextMenuRequested, [=](auto pt) {
        if (QListWidgetItem* item = d->ui.memberList->itemAt(pt)) {
            MemberMenu::showAt(this, d->repo, sessionIdFrom(d->filter), item->data(Qt::UserRole).value<Member>(),
                    d->ui.memberList->mapToGlobal(pt));
        }
    });

    connect(d->ui.registerButton, &QPushButton::clicked, [=] {
        auto dialog = new EditMemberDialog(d->repo, this);
        connect(dialog, &EditMemberDialog::newMemberCreated, [this](auto id) {
            emit this->memberSelected(id);
            if (d->closeWhenSelected) QDialog::accept();
        });
        dialog->show();
    });

    connect(d->ui.memberList, &QListWidget::itemDoubleClicked, [=](auto item) {
        item->setSelected(true);
        accept();
    });

    reload();
    validateForm();
}

MemberSelectDialog::~MemberSelectDialog() {
    delete d;
}

void MemberSelectDialog::reload() {
    QVector<Member> members;

    if (auto needle = d->ui.filterEdit->text().trimmed(); !needle.isEmpty()) {
        members = d->repo->findMember(d->filter, needle);
    } else {
        members = d->repo->getMembers(d->filter);
    }

    d->ui.memberList->clear();
    QFont font;
    font.setPointSize(20.0);
    font.setBold(true);
    for (const auto &member : members) {
        auto item = new QListWidgetItem(member.fullName(), d->ui.memberList);
        item->setFont(font);
        item->setData(Qt::UserRole, QVariant::fromValue(member));
        item->setForeground(MemberPainter::colorForMember(member));
    }
}

void MemberSelectDialog::validateForm() {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        button->setEnabled(!d->ui.memberList->selectedItems().isEmpty());
    }
}

void MemberSelectDialog::accept() {
    auto items = d->ui.memberList->selectedItems();
    auto currentItem = items.isEmpty() ? nullptr : items.first();
    if (!currentItem) {
        return;
    }

    if (auto data = currentItem->data(Qt::UserRole); data.isValid()) {
        emit this->memberSelected(data.value<Member>().id);
        if (d->closeWhenSelected) QDialog::accept();
        else {
            d->ui.memberList->clearSelection();
        }
    }
}

void MemberSelectDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void MemberSelectDialog::setAcceptButtonText(const QString &text) {
    if (auto button = d->ui.buttonBox->button(QDialogButtonBox::Ok)) {
        button->setText(text);
    }
}

void MemberSelectDialog::clearFilter() {
    d->ui.filterEdit->clear();
}
