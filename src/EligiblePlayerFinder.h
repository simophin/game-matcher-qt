//
// Created by Fanchao Liu on 14/08/20.
//

#ifndef GAMEMATCHER_ELIGIBLEPLAYERFINDER_H
#define GAMEMATCHER_ELIGIBLEPLAYERFINDER_H

#include "PlayerInfo.h"
#include "models.h"

#include <vector>
#include "span.h"

class GameStats;

class EligiblePlayerFinder {
public:
    static std::vector<PlayerInfo> getEligiblePlayers(
            nonstd::span<const Member> members, int numMaxSeats, const GameStats &stats, int randomSeed);
};


#endif //GAMEMATCHER_ELIGIBLEPLAYERFINDER_H
