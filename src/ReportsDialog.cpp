//
// Created by Fanchao Liu on 12/08/20.
//

#include "ReportsDialog.h"
#include "ui_ReportsDialog.h"

#include <QEvent>
#include <QFileDialog>
#include <QMessageBox>

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

        connect(d->currentReport.get(), &BaseReport::dataChanged, this, &ReportsDialog::reload);

        auto requirement = d->currentReport->sessionRequirement();
        auto sessionRequired = requirement.min || requirement.max;
        d->ui.selectedSessionLabel->setEnabled(sessionRequired);
        d->ui.selectSessionButton->setEnabled(sessionRequired);
        d->ui.sessionsLabel->setEnabled(sessionRequired);
    });

    connect(d->ui.selectSessionButton, &QPushButton::clicked, [=] {
        auto dialog = new SessionSelectionDialog(d->repo, d->currentReport->sessionRequirement(), this);
        connect(dialog, &SessionSelectionDialog::onSessionSelected, [=](auto sessionIds) {
            if (d->currentReport) {
                d->currentReport->setSessions(sessionIds);
                d->ui.selectedSessionLabel->setText(tr("%1 selected").arg(sessionIds.size()));
            }
        });
        dialog->show();
    });

    connect(d->ui.closeButton, &QPushButton::clicked, this, &ReportsDialog::close);
    connect(d->ui.exportButton, &QPushButton::clicked, [=] {
        auto fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV..."));
        if (fileName.isEmpty()) return;

        if (!fileName.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
            fileName += QStringLiteral(".csv");
        }

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to save to this file"));
            return;
        }

        QTextStream stream(&file);

        for (int i = 0, size = d->ui.dataTable->columnCount(); i < size; i++) {
            stream << d->ui.dataTable->horizontalHeaderItem(i)->text();
            if (i < size - 1) {
                stream << ",";
            }
        }

        stream << "\n";

        for (int i = 0, numRow = d->ui.dataTable->rowCount(); i < numRow; i++) {
            for (int j = 0, numColumns = d->ui.dataTable->columnCount(); j < numColumns; j++) {
                if (auto item = d->ui.dataTable->item(i, j)) {
                    stream << item->text();
                }

                if (j < numColumns - 1) {
                    stream << ",";
                }
            }

            stream << "\n";
        }

        QMessageBox::information(this, tr("Success"), tr("Successfully saved to %1").arg(fileName));
    });
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

void ReportsDialog::reload() {
    d->ui.dataTable->clear();

    if (auto report = d->currentReport.get()) {
        auto columnNames = report->columnNames();

        d->ui.dataTable->setRowCount(report->numRows());
        d->ui.dataTable->setColumnCount(columnNames.size());
        d->ui.dataTable->setHorizontalHeaderLabels(columnNames);

        int currRow = 0;
        report->forEachRow([&](const QVector<QVariant> &columns) {
            for (int currColumn = 0, numColumns = columns.size(); currColumn < numColumns; currColumn++) {
                d->ui.dataTable->setItem(currRow, currColumn, new QTableWidgetItem(columns[currColumn].toString()));
            }
            currRow++;
            return true;
        });

        d->ui.dataTable->resizeColumnsToContents();
        d->ui.exportButton->setEnabled(true);
    } else {
        d->ui.exportButton->setEnabled(false);
    }
}
