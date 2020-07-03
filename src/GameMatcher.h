#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include "models.h"

#include <QVector>
#include <QFuture>

class MemberInfo;

class GameMatcher {
public:
    static QFuture<QVector<GameAllocation>> match(
            const QVector<GameAllocation> &pastAllocation,
            const QVector<MemberInfo> &eligiblePlayers,
            const QVector<CourtId> &courts,
            int playerPerCourt,
            int seed);
};

#endif // GAMEMATCHER_H
