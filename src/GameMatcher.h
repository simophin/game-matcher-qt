#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include "models.h"

#include <QVector>
#include <QFuture>

class GameMatcher {
public:
    static QVector<GameAllocation> match(
            const QVector<GameAllocation> &pastAllocation,
            const QVector<Member> &eligiblePlayers,
            QVector<CourtId> courts,
            size_t playerPerCourt,
            int seed);
};

#endif // GAMEMATCHER_H
