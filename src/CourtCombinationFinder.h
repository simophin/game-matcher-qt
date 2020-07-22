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
    CourtCombinationFinder(const GameStats &stats, int numPlayersPerCourt, unsigned int minLevel, unsigned int maxLevel)
            : stats_(stats), numPlayersPerCourt_(numPlayersPerCourt), minLevel_(minLevel), maxLevel_(maxLevel) {}

    std::vector<GameAllocation> find(nonstd::span<const CourtId> courts, nonstd::span<const PlayerInfo> players);

protected:
    struct CourtAllocation {
        std::vector<PlayerInfo> players;
        int quality;
    };
    virtual std::vector<CourtAllocation> doFind(nonstd::span<const PlayerInfo>, size_t numCourtAvailable) const = 0;

    GameStats const &stats_;
    int const numPlayersPerCourt_;
    unsigned int const minLevel_, maxLevel_;
};


#endif //GAMEMATCHER_COURTCOMBINATIONFINDER_H
