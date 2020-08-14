//
// Created by Fanchao Liu on 16/07/20.
//

#include "CourtCombinationFinder.h"
#include "PlayerInfo.h"

QVector<GameAllocation> CourtCombinationFinder::find(const QVector<CourtId> &courts, const QVector<PlayerInfo> &players) {
    QVector<GameAllocation> result;

    auto allocations = doFind(players, courts.size());
    auto courtId = courts.begin();
    auto allocation = allocations.begin();

    while (courtId != courts.end() && allocation != allocations.end()) {
        for (auto p : allocation->players) {
            result.push_back(GameAllocation(0, *courtId, p.memberId, allocation->quality));
        }
        courtId++;
        allocation++;
    }

    return result;
}
