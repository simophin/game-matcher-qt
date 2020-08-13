//
// Created by Fanchao Liu on 12/08/20.
//

#include "ReportsDialog.h"
#include "ui_ReportsDialog.h"

#include <QEvent>

#include "MembersPaymentReport.h"
#include "SessionSelectionDialog.h"


struct PaymentReportFactory {
    QString name;
    std::function<BaseReport *(ClubRepository *, QObject *)> factory;
};

Q_DECLARE_METATYPE(const PaymentReportFactory *);

static const PaymentReportFactory paymentReportFactories[] = {
        {
                QStringLiteral("Member payment history"),
                [](auto repo, auto parent) {
                    return new MembersPaymentReport(repo, parent);
                }
        }
};


struct ReportsDialog::Impl {
    ClubRepository * const repo;
    Ui::ReportsDialog ui;
    std::unique_ptr<BaseReport> currentReport;

    QSet<SessionId> selectedSession;

    void loadData() {
        currentReport->setSessions(selectedSession);
        ui.selectedSessionLabel->setText(tr("%1 selected").arg(selectedSession.size()));

        ui.dataTable->clear();
        auto columnNames = currentReport->columnNames();

        ui.dataTable->setRowCount(currentReport->numRows());
        ui.dataTable->setColumnCount(columnNames.size());

        ui.dataTable->setHorizontalHeaderLabels(columnNames);

        int currRow = 0;
        currentReport->forEachRow([&](const QVector<QVariant> &columns) {
            for (int currColumn = 0, numColumns = columns.size(); currColumn < numColumns; currColumn++) {
                ui.dataTable->setItem(currRow, currColumn, new QTableWidgetItem(columns[currColumn].toString()));
            }
            currRow++;
            return true;
        });

        ui.dataTable->resizeColumnsToContents();
    }
};

ReportsDialog::ReportsDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    qRegisterMetaType<const PaymentReportFactory *>();

    for (auto &factory : paymentReportFactories) {
        d->ui.reportTypeComboBox->addItem(factory.name, QVariant::fromValue(&factory));
    }

    connect(d->ui.reportTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int currentIndex) {
        if (currentIndex < 0) return;

        auto factory = d->ui.reportTypeComboBox->itemData(currentIndex).value<const PaymentReportFactory *>();
        d->currentReport.reset(factory->factory(repo, nullptr));

        auto requirement = d->currentReport->sessionRequirement();
        auto sessionRequired = requirement.min || requirement.max;
        d->ui.selectedSessionLabel->setEnabled(sessionRequired);
        d->ui.selectSessionButton->setEnabled(sessionRequired);
        d->ui.sessionsLabel->setEnabled(sessionRequired);
    });

    connect(d->ui.selectSessionButton, &QPushButton::clicked, [=] {
        auto dialog = new SessionSelectionDialog(d->repo, d->currentReport->sessionRequirement(), this);
        connect(dialog, &SessionSelectionDialog::onSessionSelected, [=](auto sessionIds) {
            d->selectedSession = sessionIds;
            d->loadData();
        });
        dialog->show();
    });

    connect(d->ui.closeButton, &QPushButton::clicked, this, &ReportsDialog::close);
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
