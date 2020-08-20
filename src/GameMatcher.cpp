#include "GameMatcher.h"

#include <QtDebug>
#include <QHash>

#include <algorithm>

#include "GameStats.h"
#include "BFCombinationFinder.h"
#include "SortingLevelCombinationFinder.h"
#include "EligiblePlayerFinder.h"

QVector<GameAllocation>
GameMatcher::match(const QVector<GameAllocation> &pastAllocations,
                   const QVector<Member> &allPlayers,
                   const QVector<CourtId> &courtIds,
                   unsigned playerPerCourt,
                   int seed) {
    qDebug() << "Matching using " << pastAllocations.size() << " past allocations, " << allPlayers.size()
             << " players and "
             << courtIds.size() << " courts";

    std::unique_ptr<CombinationFinder> finder;
    std::unique_ptr<GameStats> stats;

    QVector<BasePlayerInfo> players;
    players.reserve(allPlayers.size());
    for (const auto &p : allPlayers) {
        players.push_back(BasePlayerInfo(p));
    }

    if (pastAllocations.isEmpty()) {
        finder = std::make_unique<SortingLevelCombinationFinder>(playerPerCourt, seed);
    } else {
        stats = std::make_unique<GameStatsImpl>(pastAllocations);
        finder = std::make_unique<BFCombinationFinder>(playerPerCourt, *stats);
    }

    return finder->find(
            courtIds,
            EligiblePlayerFinder::findEligiblePlayers(
                    players, playerPerCourt, courtIds.size(), stats.get(), seed));
}

