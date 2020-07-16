//
// Created by Fanchao Liu on 16/07/20.
//

#include "SortingCourtCombinationFinder.h"

#include "PlayerInfo.h"

#include <algorithm>
#include <random>

struct PlayerData {
    const PlayerInfo *player;
    int index;

    inline auto operator->() const { return player; }

    PlayerData(const PlayerInfo &player, int index)
            : player(&player), index(index) {}
};

std::vector<CourtCombinationFinder::CourtAllocation>
SortingCourtCombinationFinder::doFind(nonstd::span<const PlayerInfo> playerSpan, size_t numCourtAvailable) const {
    std::vector<PlayerData> players;
    players.reserve(playerSpan.size());

    for (size_t i = 0, size = playerSpan.size(); i < size; i++) {
        players.emplace_back(playerSpan[i], i);
    }

    std::shuffle(players.begin(), players.end(), std::default_random_engine());
    std::sort(players.begin(), players.end(), [](auto &a, auto &b) {
        return b->level < a->level;
    });

    return {};
}
