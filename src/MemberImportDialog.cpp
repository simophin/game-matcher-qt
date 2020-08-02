//
// Created by Fanchao Liu on 2/08/20.
//

#include "MemberImportDialog.h"
#include "ui_MemberImportDialog.h"

#include "models.h"
#include "ClubRepository.h"

#include <QEvent>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <functional>
#include <memory>

static const auto propColumnName = "column_name";

typedef void(*PropertyWriterFn)(Member &, const QString &);

struct PropertyWriter {
    QString columnName;
    PropertyWriterFn writer;
};

static const PropertyWriter firstNameWriter = {
        QObject::tr("First name"),
        [](Member &m, const QString &item) {
            m.firstName = item;
        }
};

static const PropertyWriter lastNameWriter = {
        QObject::tr("Last name"),
        [](Member &m, const QString &item) {
            m.lastName = item;
        }
};

static const PropertyWriter genderWriter = {
        QObject::tr("Gender"),
        [](Member &m, const QString &item) {
            m.gender = (item.compare(QStringLiteral("M"), Qt::CaseInsensitive) == 0) ? Member::Male : Member::Female;
        }
};

static const PropertyWriter levelWriter = {
        QObject::tr("Level"),
        [](Member &m, const QString &item) {
            bool ok;
            m.level = item.toInt(&ok);
            if (!ok) {
                qDebug() << "Unable to convert level " << item << " to integer";
            }
        }
};

const static std::vector<const PropertyWriter *> allWriters = {
        &firstNameWriter, &lastNameWriter, &genderWriter, &levelWriter,
};

struct MemberImportDialog::Impl {
    ClubRepository *const repo;
    Ui::MemberImportDialog ui;

    void validateForm();

    QHash<QString, const PropertyWriter *> getPropertyMap() {
        QHash<QString, const PropertyWriter *> rc;
        for (int i = 0, size = ui.mappingForm->rowCount(); i < size; i++) {
            auto item = ui.mappingForm->itemAt(i, QFormLayout::FieldRole);
            if (auto box = qobject_cast<QComboBox *>(item->widget())) {
                if (auto writer = reinterpret_cast<const PropertyWriter *>(box->currentData().toLongLong())) {
                    rc[box->property(propColumnName).toString()] = writer;
                }
            }
        }
        return rc;
    }
};

static QStringList readHeaders(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) return {};

    QTextStream stream(&file);
    return stream.readLine(512).split(QStringLiteral(","));
}

static std::function<bool(Member &)>
readAll(const QString &filePath, const QHash<QString, const PropertyWriter *> &propMaps) {
    struct Context {
        QFile file;
        QTextStream stream;
        QStringList headerNames;
        QString line;
    };

    auto ctx = std::make_shared<Context>();
    ctx->file.setFileName(filePath);
    if (!ctx->file.open(QFile::ReadOnly)) return nullptr;

    ctx->stream.setDevice(&ctx->file);
    ctx->headerNames = ctx->stream.readLine(512).split(QStringLiteral(","));

    return [ctx, propMaps](Member &m) -> bool {
        if (!ctx->stream.readLineInto(&ctx->line, 512)) return false;

        auto items = ctx->line.split(QStringLiteral(","));
        for (size_t i = 0, size = std::min(ctx->headerNames.size(), items.size()); i < size; i++) {
            if (auto prop = propMaps[ctx->headerNames[i]]) {
                prop->writer(m, items[i]);
            }
        }
        return true;
    };
}

MemberImportDialog::MemberImportDialog(ClubRepository *repo, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    d->ui.pathLabel->setVisible(false);

    d->ui.mappingLabel->setVisible(false);

    d->validateForm();

    connect(d->ui.browseButton, &QPushButton::clicked, [=] {
        auto file = QFileDialog::getOpenFileName(this, tr("Select a CSV file"),
                                                 QString(), QStringLiteral("CSV Files (*.csv)"));

        if (!file.isEmpty() && file != d->ui.pathLabel->text()) {
            d->ui.mappingLabel->setVisible(true);

            while (!d->ui.mappingForm->isEmpty()) {
                d->ui.mappingForm->removeRow(0);
            }

            d->ui.pathLabel->setText(file);
            d->ui.pathLabel->setVisible(!file.isEmpty());

            for (const auto &columnName : readHeaders(file)) {
                auto box = new QComboBox();
                box->setEditable(false);
                box->addItem(tr("Ignore"), static_cast<const PropertyWriter *>(nullptr));
                for (const auto &writer : allWriters) {
                    box->addItem(writer->columnName, reinterpret_cast<qlonglong>(writer));
                    if (columnName.compare(writer->columnName, Qt::CaseInsensitive) == 0) {
                        box->setCurrentIndex(box->count() - 1);
                    }
                }
                box->setProperty(propColumnName, columnName);
                d->ui.mappingForm->addRow(columnName, box);

                connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, &MemberImportDialog::reload);
            }

            reload();
        }
    });
}

MemberImportDialog::~MemberImportDialog() {
    delete d;
}

void MemberImportDialog::Impl::validateForm() {
    if (auto button = ui.buttonBox->button(QDialogButtonBox::Save)) {
        button->setEnabled(!ui.pathLabel->text().isEmpty());
    }
}

void MemberImportDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void MemberImportDialog::reload() {
    QString filePath = d->ui.pathLabel->text();
    auto propMaps = d->getPropertyMap();

    auto watcher = new QFutureWatcher<std::vector<Member>>(this);
    watcher->setFuture(QtConcurrent::run([filePath, propMaps]() -> std::vector<Member> {
        std::vector<Member> result;
        auto fn = readAll(filePath, propMaps);
        size_t i = 0;
        while (fn(result.emplace_back()) && i++ < 20) {}
        return result;
    }));

    connect(watcher, &QFutureWatcherBase::finished, [watcher, this] {
        auto members = watcher->result();
        d->ui.previewTable->clear();
        d->ui.previewTable->setRowCount(members.size());
        d->ui.previewTable->setColumnCount(4);
        d->ui.previewTable->setHorizontalHeaderLabels(
                {
                        tr("First name"),
                        tr("Last name"),
                        tr("Gender"),
                        tr("Level"),
                });

        int row = 0;
        for (const auto &m : members) {
            d->ui.previewTable->setItem(row, 0, new QTableWidgetItem(m.firstName));
            d->ui.previewTable->setItem(row, 1, new QTableWidgetItem(m.lastName));
            d->ui.previewTable->setItem(row, 2,
                                        new QTableWidgetItem(m.gender == Member::Male ? tr("M") : tr("F")));
            d->ui.previewTable->setItem(row, 3,
                                        new QTableWidgetItem(QString::number(m.level)));
            row++;
        }
    });

    d->validateForm();
}

void MemberImportDialog::accept() {
    QVector<Member> failure;
    auto numSuccess = d->repo->importMembers(readAll(d->ui.pathLabel->text(), d->getPropertyMap()), failure);
    auto body = tr("Number of success imports: %1").arg(numSuccess);
    if (!failure.isEmpty()) {
        body += tr("\nFailed imports:");
        int num = 0;
        for (const auto &m : failure) {
            body += tr("\n%1").arg(m.fullName());
            if (++num > 20) break;
        }
        body += tr("\n...");
    }
    QMessageBox::information(this, tr("Import result"), body);
    QDialog::accept();
}
