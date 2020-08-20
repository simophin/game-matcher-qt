//
// Created by Fanchao Liu on 14/08/20.
//

#ifndef GAMEMATCHER_ELIGIBLEPLAYERFINDER_H
#define GAMEMATCHER_ELIGIBLEPLAYERFINDER_H

#include "PlayerInfo.h"
#include "models.h"

#include <QVector>

class GameStats;

class EligiblePlayerFinder {
public:
    static QVector<PlayerInfo> findEligiblePlayers(const QVector<BasePlayerInfo> &members, unsigned playerPerCourt,
                                                   unsigned numCourt, const GameStats *stats);
};


#endif //GAMEMATCHER_ELIGIBLEPLAYERFINDER_H
