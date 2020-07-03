#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include <QObject>
#include <QVector>

#include "models.h"

class MemberInfo;

class GameMatcher : public QObject {
Q_OBJECT
public:
    explicit GameMatcher(QObject *parent = nullptr): QObject(parent) {}

public slots:
    QVector<GameAllocation> match(
            const QVector<GameAllocation> &pastAllocation,
            const QVector<MemberInfo> &eligiblePlayers,
            const QVector<CourtId> &courts,
            int playerPerCourt,
            int seed) const;
};

#endif // GAMEMATCHER_H
