//
// Created by Fanchao Liu on 15/08/20.
//

#include "SortingLevelCombinationFinder.h"

#include <random>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/action.hpp>

static bool comparePlayerInfoAsc(const PlayerInfo &lhs, const PlayerInfo &rhs) {
    return lhs.level < rhs.level;
}

static bool comparePlayerInfoDesc(const PlayerInfo &lhs, const PlayerInfo &rhs) {
    return rhs.level < lhs.level;
}

QVector<SortingLevelCombinationFinder::CourtAllocation>
SortingLevelCombinationFinder::doFind(const QVector<PlayerInfo> &players, unsigned int numCourtAvailable) const {
    if (players.isEmpty() || numCourtAvailable == 0 || numPlayersPerCourt_ == 0) {
        return {};
    }

    using namespace ranges;

    auto comparator = (sorting_ == Sorting::ASC) ? &comparePlayerInfoAsc : &comparePlayerInfoDesc;
    unsigned numOnCourt = std::min(numCourtAvailable, players.size() / numPlayersPerCourt_) * numPlayersPerCourt_;

    auto topPlayers = players
                      | to<QVector<PlayerInfo>>()
                      | actions::shuffle(std::default_random_engine(randomSeed_))
                      | actions::stable_sort(comparator)
                      | actions::take(numOnCourt);


    return topPlayers
           | views::chunk(numPlayersPerCourt_)
           | views::transform(
            [](auto players) {
                return CourtAllocation{players | to<QVector<PlayerInfo>>(), 100};
            })
           | to<QVector<CourtAllocation>>();
}

