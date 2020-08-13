//
// Created by Fanchao Liu on 12/08/20.
//

#include "SessionSelectionDialog.h"
#include "ui_SessionSelectionDialog.h"

#include "ClubRepository.h"

#include <QEvent>
#include <QPushButton>

struct SessionSelectionDialog::Impl {
    const SizeRange range;
    ClubRepository * const repo;
    Ui::SessionSelectionDialog ui;
};

SessionSelectionDialog::SessionSelectionDialog(ClubRepository *repo, SizeRange range, QWidget *parent)
        : QDialog(parent), d(new Impl{range, repo}) {
    d->ui.setupUi(this);

    if (range.min && range.max) {
        d->ui.requirementLabel->setText(tr("You need to select at least %1 and at most %2 sessions").arg(
                QString::number(*range.min),
                QString::number(*range.max)));
    } else if (!range.min && !range.max) {
        d->ui.requirementLabel->setText(tr("Select any session"));
    } else if (range.min) {
        d->ui.requirementLabel->setText(tr("Select at least %1 session(s)").arg(*range.min));
    } else {
        d->ui.requirementLabel->setText(tr("Select at most %1 session(s)").arg(*range.max));
    }

    d->ui.sessionList->setSelectionMode((range.max && *range.max < 2) ? QAbstractItemView::SingleSelection : QAbstractItemView::MultiSelection);

    connect(d->ui.sessionList, &QListWidget::itemSelectionChanged, this, &SessionSelectionDialog::validateForm);
    validateForm();
    reload();
}

SessionSelectionDialog::~SessionSelectionDialog() {
    delete d;
}

void SessionSelectionDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void SessionSelectionDialog::validateForm() {
    auto selected = d->ui.sessionList->selectedItems();
    bool valid = true;

    // Verify min
    valid &= (!d->range.min) || (selected.size() >= d->range.min);

    // Verify max
    valid &= (!d->range.max) || (selected.size() <= d->range.max);

    d->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void SessionSelectionDialog::accept() {
    QSet<SessionId> selectedIds;
    for (auto item : d->ui.sessionList->selectedItems()) {
        selectedIds.insert(item->data(Qt::UserRole).value<SessionId>());
    }
    emit onSessionSelected(selectedIds);

    QDialog::accept();
}

void SessionSelectionDialog::reload() {
    d->ui.sessionList->clear();
    for (const auto &s : d->repo->getAllSessions()) {
        auto item = new QListWidgetItem(s.startTime.toString(DATE_TIME_FORMAT));
        item->setData(Qt::UserRole, s.id);
        d->ui.sessionList->addItem(item);
    }

}
