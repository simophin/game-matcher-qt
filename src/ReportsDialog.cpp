//
// Created by Fanchao Liu on 12/08/20.
//

#include "ReportsDialog.h"
#include "ui_ReportsDialog.h"

#include <QEvent>

#include "MembersPaymentReport.h"


struct PaymentReportFactory {
    QString name;
    std::function<BaseReport *(ClubRepository *, QObject *)> factory;
};

static const PaymentReportFactory paymentReportFactories[] = {
        {
                QStringLiteral("Member payment history"),
                [](auto repo, auto parent) {
                    return new MembersPaymentReport(repo, parent);
                }
        }
};


struct ReportsDialog::Impl {
    Ui::ReportsDialog ui;
};

ReportsDialog::ReportsDialog(QWidget *parent)
        : QDialog(parent), d(new Impl) {
    d->ui.setupUi(this);

    for (auto &factory : paymentReportFactories) {
        d->ui.reportTypeComboBox->addItem(factory.name, QVariant::fromValue(&factory));
    }
}

ReportsDialog::~ReportsDialog() {
    delete d;
}

void ReportsDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}
