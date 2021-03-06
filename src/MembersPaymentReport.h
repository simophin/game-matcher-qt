//
// Created by Fanchao Liu on 12/08/20.
//

#ifndef GAMEMATCHER_MEMBERSPAYMENTREPORT_H
#define GAMEMATCHER_MEMBERSPAYMENTREPORT_H

#include "BaseReport.h"

class MembersPaymentReport : public BaseReport {
public:
    MembersPaymentReport(ClubRepository *repo, QObject *parent);

    virtual ~MembersPaymentReport();

    SizeRange sessionRequirement() const override {
        return SizeRange(1, std::nullopt);
    }

    void forEachRow(RowCallback) override;
    void setSessions(const QSet<SessionId> &set) override;
    size_t numRows() const override;

    QStringList columnNames() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};


#endif //GAMEMATCHER_MEMBERSPAYMENTREPORT_H
