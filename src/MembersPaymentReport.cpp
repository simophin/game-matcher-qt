//
// Created by Fanchao Liu on 12/08/20.
//

#include "MembersPaymentReport.h"

#include "ClubRepository.h"

#include <optional>
#include <QHash>
#include <QVector>
#include <QMap>


struct MembersPaymentReport::Impl {
    struct SessionPaymentHistory {
        QDateTime startTime;
        QHash<MemberId, bool> history;
    };

    ClubRepository * const repo;
    QSet<SessionId> sessions;
    bool dataDirty = true;
    QMap<MemberId, QString> members;
    QHash<SessionId, SessionPaymentHistory> paymentHistory;

    void loadDataIfNecessary() {
        if (!dataDirty) return;

        dataDirty = false;
        members.clear();
        paymentHistory.clear();
        if (sessions.isEmpty()) return;

        for (const auto &record : repo->getPaymentRecords(sessions)) {
            paymentHistory[record.sessionId].startTime.setSecsSinceEpoch(record.sessionStartTime);
            paymentHistory[record.sessionId].history[record.memberId] = record.paid;
            if (!members.contains(record.memberId)) {
                members[record.memberId] = tr("%1 %2").arg(record.memberFirstName, record.memberLastName);
            }
        }
    }
};

MembersPaymentReport::MembersPaymentReport(ClubRepository *repo, QObject *parent) : BaseReport(repo, parent), d(new Impl{ repo }) {}

MembersPaymentReport::~MembersPaymentReport() = default;

void MembersPaymentReport::setSessions(const QSet<SessionId> &sessions) {
    if (d->sessions != sessions) {
        d->sessions = sessions;
        d->dataDirty = true;
        emit this->dataChanged();
    }
}

void MembersPaymentReport::forEachRow(BaseReport::RowCallback cb) {
    d->loadDataIfNecessary();

    QVector<QVariant> columns(d->paymentHistory.size() + 2);

    for (auto memberIter = d->members.begin(); memberIter != d->members.end(); ++memberIter) {
        columns[0] = memberIter.value();

        int i = 2;
        int totalUnpaid = 0;
        for (auto iter = d->paymentHistory.begin(); iter != d->paymentHistory.end(); ++iter) {
            auto paid = iter->history.find(memberIter.key());
            if (paid == iter->history.end()) {
                columns[i++] = tr("N/A");
            } else if (paid.value()) {
                columns[i++] = tr("Paid");
            } else {
                columns[i++] = tr("Unpaid");
                totalUnpaid++;
            }
        }
        columns[1] = totalUnpaid;

        if (!cb(columns)) {
            break;
        }
    }

    // Empty line
    if (!cb({})) return;

    // Member played
    {
        columns[0] = tr("Total member played");
        columns[1].clear();

        int i = 2;
        for (auto iter = d->paymentHistory.begin(); iter != d->paymentHistory.end(); ++iter) {
            columns[i++] = QString::number(iter->history.size());
        }
        if (!cb(columns)) return;
    }

    // Member paid
    {
        columns[0] = tr("Total paid");
        columns[1].clear();

        int i = 2;
        for (auto iter = d->paymentHistory.begin(); iter != d->paymentHistory.end(); ++iter) {
            columns[i++] = QString::number(std::count_if(iter->history.begin(), iter->history.end(), [&](bool paid) {
                return paid;
            }));
        }

        if (!cb(columns)) return;
    }

    // Member unpaid
    {
        columns[0] = tr("Total unpaid");
        columns[1].clear();

        int i = 2;
        for (auto iter = d->paymentHistory.begin(); iter != d->paymentHistory.end(); ++iter) {
            columns[i++] = QString::number(std::count_if(iter->history.begin(), iter->history.end(), [&](bool paid) {
                return !paid;
            }));
        }

        if (!cb(columns)) return;
    }
}

QStringList MembersPaymentReport::columnNames() const {
    d->loadDataIfNecessary();

    QStringList names;
    names.reserve(d->paymentHistory.size() + 2);
    names.append(tr("Name"));
    names.append(tr("Total unpaid"));
    for (const auto &history : d->paymentHistory) {
        names.append(history.startTime.toLocalTime().toString(DATE_TIME_FORMAT));
    }
    return names;
}

size_t MembersPaymentReport::numRows() const {
    d->loadDataIfNecessary();
    return d->members.size() + 4;
}
