//
// Created by Fanchao Liu on 14/08/20.
//

#include "MemberListDialog.h"
#include "ui_MemberListDialog.h"
#include "ClubRepository.h"
#include <TypeUtils.h>

#include <QEvent>
#include <functional>

struct ColumnDeclaration {
    QString name;
    std::function<QString(const BaseMember &m)> readFunc;
    std::function<bool(BaseMember &m, QString)> writeFunc;
};

static const ColumnDeclaration COLUMNS[] = {
        {
            QObject::tr("First name"),
            [](const BaseMember &m) { return m.firstName; },
            [](BaseMember &m, QString value) {
                value = value.trimmed();
                if (value.isEmpty()) return false;
                m.firstName = value;
                return true;
            },
        },
        {
                QObject::tr("Last name"),
                [](const BaseMember &m) { return m.lastName; },
                [](BaseMember &m, QString value) {
                    value = value.trimmed();
                    if (value.isEmpty()) return false;
                    m.lastName = value;
                    return true;
                },
        },
        {
                QObject::tr("Gender"),
                [](const BaseMember &m) { return m.genderString(); },
                [](BaseMember &m, QString value) {
                    auto gender = sqlx::enumFromString<BaseMember::Gender>(value.toUtf8().data());
                    if (!gender) return false;
                    m.gender = *gender;
                    return true;
                },
        },
        {
                QObject::tr("Level"),
                [](const BaseMember &m) { return QString::number(m.level); },
                [](BaseMember &m, QString value) {
                    bool ok;
                    if (auto level = value.toInt(&ok); ok) {
                        m.level = level;
                    }
                    return ok;
                },
        },
        {
                QObject::tr("Phone"),
                [](const BaseMember &m) { return m.phone; },
                [](BaseMember &m, QString value) {
                    m.phone = value.trimmed();
                    return true;
                },
        },
        {
                QObject::tr("Email"),
                [](const BaseMember &m) { return m.email; },
                [](BaseMember &m, QString value) {
                    m.email = value.trimmed();
                    return true;
                },
        },
};

struct MemberListDialog::Impl {
    ClubRepository *const repo;
    Ui::MemberListDialog ui;
    bool isUpdatingList = false;
};

MemberListDialog::MemberListDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    QStringList labels;
    for (const auto &col : COLUMNS) {
        labels.push_back(col.name);
    }
    d->ui.memberWidget->setColumnCount(labels.size());
    d->ui.memberWidget->setHorizontalHeaderLabels(labels);

    connect(d->repo, &ClubRepository::memberChanged, this, &MemberListDialog::reload);

    connect(d->ui.memberWidget, &QTableWidget::itemChanged, [=](QTableWidgetItem * item) {
        if (d->isUpdatingList) return;
        if (!item) return;

        auto member = d->repo->getMember(item->data(Qt::UserRole).value<MemberId>());
        if (!member) return;


        auto col = COLUMNS[item->column()];
        if (!col.writeFunc(*member, item->text()) || !d->repo->saveMember(*member)) {
            auto oldUpdating = d->isUpdatingList;
            d->isUpdatingList = true;
            item->setText(col.readFunc(*member));
            d->isUpdatingList = oldUpdating;
        }
    });

    reload();
}

MemberListDialog::~MemberListDialog() {
    delete d;
}

void MemberListDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void MemberListDialog::reload() {
    auto oldUpdating = d->isUpdatingList;
    d->isUpdatingList = true;
    d->ui.memberWidget->clearContents();

    auto members = d->repo->getMembers(AllMembers{});
    d->ui.memberWidget->setRowCount(members.size());

    for (int i = 0, numRow = d->ui.memberWidget->rowCount(); i < numRow; i++) {
        auto &m = members[i];
        int j = 0;
        for (const auto &col : COLUMNS) {
            auto item = new QTableWidgetItem();
            item->setData(Qt::UserRole, m.id);
            item->setText(col.readFunc(m));
            d->ui.memberWidget->setItem(i, j++, item);
        }
    }

    d->ui.memberWidget->resizeColumnsToContents();
    d->isUpdatingList = oldUpdating;
}
