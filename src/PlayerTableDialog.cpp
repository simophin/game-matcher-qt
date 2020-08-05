//
// Created by Fanchao Liu on 5/08/20.
//

#include "PlayerTableDialog.h"
#include "ui_PlayerTableDialog.h"

#include <QEvent>

struct PlayerTableDialog::Impl {
    Ui::PlayerTableDialog ui;
};

PlayerTableDialog::PlayerTableDialog(ClubRepository *repo, SessionId id, QWidget *parent)
        : QDialog(parent), d(new Impl) {
    d->ui.setupUi(this);

    d->ui.widget->load(id, repo);

    connect(d->ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(d->ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

PlayerTableDialog::~PlayerTableDialog() {
    delete d;
}

void PlayerTableDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}
