//
// Created by Fanchao Liu on 16/07/20.
//

#include "SortingCourtCombinationFinder.h"
#include "PlayerInfo.h"

#include <algorithm>
#include <random>


std::vector<CourtCombinationFinder::CourtAllocation>
SortingCourtCombinationFinder::doFind(nonstd::span<const PlayerInfo> playerSpan, size_t numCourtAvailable) const {
    std::vector<const PlayerInfo*> players;
    players.reserve(playerSpan.size());

    for (const auto &item : playerSpan) {
        players.push_back(&item);
    }

    std::shuffle(players.begin(), players.end(), std::default_random_engine());
    std::stable_sort(players.begin(), players.end(), [](auto &a, auto &b) {
        return b->level < a->level;
    });

    return {};
}
