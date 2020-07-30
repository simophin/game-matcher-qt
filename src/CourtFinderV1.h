//
// Created by Fanchao Liu on 30/07/20.
//

#ifndef GAMEMATCHER_COURTFINDERV1_H
#define GAMEMATCHER_COURTFINDERV1_H

#include "CourtCombinationFinder.h"

class CourtFinderV1 : public CourtCombinationFinder {
public:
    using CourtCombinationFinder::CourtCombinationFinder;

protected:
    std::vector<CourtAllocation> doFind(nonstd::span<const PlayerInfo> span, size_t numCourtAvailable) const override;
};


#endif //GAMEMATCHER_COURTFINDERV1_H
