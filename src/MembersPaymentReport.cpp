//
// Created by Fanchao Liu on 12/08/20.
//

#include "MembersPaymentReport.h"

#include "ClubRepository.h"

#include <optional>
#include <QHash>
#include <QVector>
#include <set>


struct MembersPaymentReport::Impl {
    struct MemberEntry {
        MemberId id;
        QString name;

        inline bool operator<(const MemberEntry &rhs) const {
            return id < rhs.id;
        }
    };

    struct SessionPaymentHistory {
        QDateTime startTime;
        QHash<MemberId, bool> history;
    };

    ClubRepository * const repo;
    QSet<SessionId> sessions;
    bool dataDirty = true;
    std::set<MemberEntry> members;
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
            members.insert(MemberEntry {
                    record.memberId,
                    tr("%1 %2").arg(record.memberFirstName, record.memberLastName)
            });
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

    QVector<QVariant> columns(d->paymentHistory.size() + 1);

    for (const auto &[memberId, memberName] : d->members) {
        columns[0] = memberName;

        int i = 2;
        int totalUnpaid = 0;
        for (auto iter = d->paymentHistory.begin(); iter != d->paymentHistory.end(); ++iter) {
            auto paid = iter->history.find(memberId);
            if (paid == iter->history.end()) {
                columns[i++] = tr("N/A");
            } else if (paid.value()) {
                columns[i++] = tr("Yes");
            } else {
                columns[i++] = tr("No");
                totalUnpaid++;
            }
        }
        columns[1] = totalUnpaid;

        if (!cb(columns)) {
            break;
        }
    }
}

QStringList MembersPaymentReport::columnNames() const {
    d->loadDataIfNecessary();

    QStringList names;
    names.reserve(d->paymentHistory.size() + 2);
    names.append(tr("Name"));
    names.append(tr("Total unpaid times"));
    for (const auto &history : d->paymentHistory) {
        names.append(history.startTime.toString());
    }
    return names;
}

size_t MembersPaymentReport::numRows() const {
    d->loadDataIfNecessary();
    return d->members.size();
}
