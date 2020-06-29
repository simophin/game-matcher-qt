//
// Created by Fanchao Liu on 27/06/20.
//
#include <QMessageBox>
#include "SessionDialog.h"
#include "CourtDisplay.h"
#include "ui_SessionDialog.h"

#include "clubrepository.h"

struct SessionDialog::Impl {
    ClubRepository *repo;
    SessionData session;
    Ui::SessionDialog ui;
};

static const auto minCourtDisplayWidth = 90;
static const auto maxCourtDisplayWidth = 120;

class CourtListModel : public QAbstractListModel {
public:
    CourtListModel(QObject *parent) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent) const override {
        return 0;
    }

    int columnCount(const QModelIndex &parent) const override {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const override {
        return QVariant();
    }

private:

};

SessionDialog::SessionDialog(ClubRepository *repo, SessionId sessionId, QWidget *parent)
        : QDialog(parent), d(new Impl{repo}) {
    d->ui.setupUi(this);

    if (auto session = repo->getSession(sessionId)) {
        d->session = *session;
        onSessionDataChanged();
    } else {
        QMessageBox::warning(this, tr("Unable to open session"), tr("Please try again"));
    }
}

SessionDialog::~SessionDialog() {
    delete d;
}

void SessionDialog::onSessionDataChanged() {

}

void SessionDialog::onCurrentGameChanged() {
    auto game = d->repo->lastGameInfo();
    if (game.empty()) {
    } else {
        QVector<CourtInfo> courts;
        courts.reserve(game.courts.size());
        QSet<MemberId> sameFirstNames;
        QMap<QString, MemberId> firstNameMaps;
        for (const auto &m : d->session.checkedIn) {
            if (firstNameMaps.contains(m.firstName)) {
                sameFirstNames.insert(m.id);
                sameFirstNames.insert(firstNameMaps[m.firstName]);
            } else {
                firstNameMaps[m.firstName] = m.id;
            }
        }

        for (const auto &item : game.courts) {
            CourtInfo info;
            info.courtName = item.courtName;
            info.courtId = item.courtId;
            for (const auto &m : item.players) {
                info.players.append(CourtPlayer{
                        m.id,
                        sameFirstNames.contains(m.id)
                        ? tr("%1 %2", "Full name: First Last").arg(m.firstName, m.lastName)
                        : m.firstName
                });
            }
        }

        d->ui.courtView->setItemDelegate()
    }
}
