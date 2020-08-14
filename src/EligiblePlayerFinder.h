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
    static QVector<PlayerInfo> findEligiblePlayers(
            const QVector<BasePlayerInfo> &members,
            size_t playerPerCourt,
            size_t numCourt,
            const GameStats &stats,
            int randomSeed);
};


#endif //GAMEMATCHER_ELIGIBLEPLAYERFINDER_H
