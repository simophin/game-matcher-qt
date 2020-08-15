#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include "models.h"

#include <QVector>

class GameMatcher {
public:
    static QVector<GameAllocation>
    match(const QVector<GameAllocation> &pastAllocation,
          const QVector<Member> &members,
          const QVector<CourtId> &courts,
          unsigned playerPerCourt,
          int seed);
};

#endif // GAMEMATCHER_H
