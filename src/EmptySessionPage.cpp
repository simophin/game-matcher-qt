//
// Created by Fanchao Liu on 28/06/20.
//

#include "EmptySessionPage.h"
#include "ui_EmptySessionPage.h"

#include "ClubRepository.h"
#include "NewSessionDialog.h"
#include "FakeNames.h"
#include "MemberImportDialog.h"
#include "ReportsDialog.h"
#include "MemberListDialog.h"

#include <QRandomGenerator>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

struct EmptySessionPage::Impl {
    ClubRepository *const repo;
    Ui::EmptySessionPage ui;
};

EmptySessionPage::EmptySessionPage(ClubRepository *repo, QWidget *parent)
        : QFrame(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);
    d->ui.createFakeButton->setVisible(false);
    d->ui.checkInRandomButton->setVisible(false);
#ifndef NDEBUG
    d->ui.createFakeButton->setVisible(true);
    d->ui.checkInRandomButton->setVisible(true);
#endif

    reload();
    connect(d->repo, &ClubRepository::clubInfoChanged, this, &EmptySessionPage::reload);
    connect(d->repo, &ClubRepository::sessionChanged, this, &EmptySessionPage::reload);

    connect(d->ui.closeButton, &QPushButton::clicked, this, &EmptySessionPage::clubClosed);
    connect(d->ui.resumeButton, &QPushButton::clicked, this, &EmptySessionPage::lastSessionResumed);

    connect(d->ui.startButton, &QPushButton::clicked, [=] {
        auto dialog = new NewSessionDialog(d->repo, this);
        dialog->show();
        connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
        connect(dialog, &NewSessionDialog::sessionCreated, this, &EmptySessionPage::newSessionCreated);
    });

    connect(d->ui.exportButton, &QPushButton::clicked, [=] {
        auto csvFileName = QFileDialog::getSaveFileName(this, tr("Save as..."));
        if (csvFileName.isEmpty()) return;

        static const auto csvExt = QStringLiteral(".csv");

        if (!csvFileName.endsWith(csvExt)) {
            csvFileName.append(csvExt);
        }

        QFile csvFile(csvFileName);
        if (!csvFile.open(QFile::WriteOnly)) {
            QMessageBox::warning(
                    this, tr("Error"),
                    tr("Unable to write to this file: %1").arg(csvFile.errorString()));
            return;
        }

        auto members = d->repo->getMembers(AllMembers{});

        QTextStream stream(&csvFile);
        // Header
        stream << "First name," << "Last name," << "Level," << "Gender\n";
        for (const auto &m : members) {
            stream << m.firstName << "," << m.lastName << "," << m.level
                <<  "," << (m.gender == Member::Male ? "M" : "F") << "\n";
        }

        QMessageBox::information(this, tr("Success"),
                                 tr("Successfully exported %1 members").arg(members.size()));
    });

    connect(d->ui.importButton, &QPushButton::clicked, [=] {
        auto dialog = new MemberImportDialog(d->repo, this);
        dialog->show();
    });

    connect(d->ui.reportButton, &QPushButton::clicked, [=] {
        auto dialog = new ReportsDialog(d->repo, this);
        dialog->show();
    });

    connect(d->ui.membersButton, &QPushButton::clicked, [=] {
        auto dialog = new MemberListDialog(d->repo, this);
        dialog->show();
    });

#ifndef NDEBUG
    connect(d->ui.createFakeButton, &QPushButton::clicked, [=] {
        auto range = d->repo->getLevelRange();
        QVector<BaseMember> failed;
        auto names = FakeNames::names();
        auto iter = names.begin();
        d->repo->importMembers([&](BaseMember &m) {
             if (iter == names.end()) return false;
             auto components = iter->split(QStringLiteral(" "));
             m.firstName = components[0];
             m.lastName = components[1];
             m.gender = QRandomGenerator::global()->generate() % 4 == 0 ? Member::Female : Member::Male;
             m.level = QRandomGenerator::global()->bounded(range.min, range.max + 1);
             iter++;
             return true;
        }, &failed);
    });

    connect(d->ui.checkInRandomButton, &QPushButton::clicked, [=] {
        if (auto lastSessionId = d->repo->getLastSession()) {
            auto members = d->repo->getMembers(NonCheckedIn{*lastSessionId});
            auto size = std::min(50, members.size()) - d->repo->getMembers(CheckedIn{*lastSessionId}).size();
            std::shuffle(members.begin(), members.end(), std::default_random_engine());
            for (int i = 0; i < size; i++) {
                d->repo->checkIn(*lastSessionId, members[i].id, i % 5 != 0);
            }
        }
    });
#endif
}

EmptySessionPage::~EmptySessionPage() {
    delete d;
}

void EmptySessionPage::reload() {
    d->ui.clubNameLabel->setText(tr("Welcome to %1").arg(d->repo->getClubName()));
    d->ui.resumeButton->setEnabled(d->repo->getLastSession().has_value());
}