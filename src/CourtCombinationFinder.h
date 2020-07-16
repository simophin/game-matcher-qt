//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_COURTCOMBINATIONFINDER_H
#define GAMEMATCHER_COURTCOMBINATIONFINDER_H

#include "span.h"
#include "models.h"

class GameStats;
struct PlayerInfo;

class CourtCombinationFinder {
public:
    CourtCombinationFinder(const GameStats &stats, const int numPlayersPerCourt)
            : stats_(stats), numPlayersPerCourt_(numPlayersPerCourt) {}

    std::vector<GameAllocation> find(nonstd::span<const CourtId> courts, nonstd::span<const PlayerInfo> players);

protected:
    virtual std::vector<int> doFind(nonstd::span<const PlayerInfo>, size_t numCourtAvailable) const = 0;

    GameStats const &stats_;
    int const numPlayersPerCourt_;
};


#endif //GAMEMATCHER_COURTCOMBINATIONFINDER_H
