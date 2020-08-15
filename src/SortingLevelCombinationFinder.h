//
// Created by Fanchao Liu on 15/08/20.
//

#ifndef GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H
#define GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H

#include "CombinationFinder.h"
#include "PlayerInfo.h"

#include <algorithm>
#include <random>


class SortingLevelCombinationFinder : public CombinationFinder {
public:
    enum class Sorting {
        ASC, DESC,
    };

    SortingLevelCombinationFinder(unsigned int numPlayersPerCourt,
                                  int randomSeed,
                                  Sorting sorting = Sorting::DESC)
            : CombinationFinder(numPlayersPerCourt), randomSeed_(randomSeed), sorting_(sorting) {}

protected:
    static inline bool comparePlayerInfoAsc(const PlayerInfo &lhs, const PlayerInfo &rhs) {
        return lhs.level < rhs.level;
    }

    static inline bool comparePlayerInfoDesc(const PlayerInfo &lhs, const PlayerInfo &rhs) {
        return rhs.level < lhs.level;
    }

    QVector<CourtAllocation> doFind(const QVector<PlayerInfo> &players, unsigned int numCourtAvailable) const override {
        QVector<CourtAllocation> ret;
        if (players.isEmpty() || numCourtAvailable == 0 || numPlayersPerCourt_ == 0) {
            return ret;
        }

        auto sortedPlayers = players;
        std::shuffle(sortedPlayers.begin(), sortedPlayers.end(),
                     std::default_random_engine(randomSeed_));
        std::stable_sort(sortedPlayers.begin(), sortedPlayers.end(),
                         (sorting_ == Sorting::ASC) ? &comparePlayerInfoAsc : &comparePlayerInfoDesc);

        unsigned numPlayerOnCourt = players.size() / numCourtAvailable * numPlayersPerCourt_;
        unsigned numCourt = numPlayerOnCourt / numPlayersPerCourt_;
        ret.reserve(numCourt);

        for (unsigned i = 0; i < numPlayerOnCourt; i++) {
            auto &court = ret[i / numPlayersPerCourt_];
            court.players.push_back(sortedPlayers[i]);
            court.quality = 100;
        }

        return ret;
    }

private:
    int const randomSeed_;
    Sorting const sorting_;

};


#endif //GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H
