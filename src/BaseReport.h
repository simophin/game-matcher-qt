//
// Created by Fanchao Liu on 12/08/20.
//

#ifndef GAMEMATCHER_BASEREPORT_H
#define GAMEMATCHER_BASEREPORT_H

#include <QObject>
#include <functional>
#include <optional>

#include "models.h"

#include "NumericRange.h"

class ClubRepository;

class BaseReport : public QObject {
    Q_OBJECT
public:
    BaseReport(ClubRepository *repo, QObject *parent): repo_(repo), QObject(parent) {}

    typedef std::function<bool(const QVector<QVariant> &)> RowCallback;

    virtual QStringList columnNames() const = 0;
    virtual void forEachRow(RowCallback) = 0;
    virtual SizeRange sessionRequirement() const = 0;
    virtual void setSessions(const QSet<SessionId> &) = 0;
    virtual size_t numRows() const = 0;

signals:
    void dataChanged();

protected:
    ClubRepository * const repo_;
};


#endif //GAMEMATCHER_BASEREPORT_H
