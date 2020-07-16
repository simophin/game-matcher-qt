//
// Created by Fanchao Liu on 16/07/20.
//

#include "CourtCombinationFinder.h"
#include "PlayerInfo.h"

std::vector<GameAllocation> CourtCombinationFinder::find(nonstd::span<const CourtId> courts, nonstd::span<const PlayerInfo> players) {
    std::vector<GameAllocation> result;

    auto found = doFind(players, courts.size());
    auto courtId = courts.begin();
    result.reserve(found.size());
    for (size_t i = 0, size = found.size(); i < size && courtId != courts.end(); i++) {
        result.emplace_back(0, *courtId, players[found[i]].memberId);
        if ((i + 1) % numPlayersPerCourt_ == 0) {
            courtId++;
        }
    }

    return result;
}
