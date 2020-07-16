//
// Created by Fanchao Liu on 16/07/20.
//

#include "CourtCombinationFinder.h"
#include "PlayerInfo.h"

std::vector<GameAllocation> CourtCombinationFinder::find(nonstd::span<const CourtId> courts, nonstd::span<const PlayerInfo> players) {
    std::vector<GameAllocation> result;

    auto allocations = doFind(players, courts.size());
    auto courtId = courts.begin();
    auto allocation = allocations.begin();

    while (courtId != courts.end() && allocation != allocations.end()) {
        for (auto p : allocation->players) {
            result.emplace_back(0, *courtId, p.memberId, allocation->quality);
        }
        courtId++;
        allocation++;
    }

    return result;
}
