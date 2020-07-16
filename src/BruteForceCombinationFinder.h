//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_BRUTEFORCECOMBINATIONFINDER_H
#define GAMEMATCHER_BRUTEFORCECOMBINATIONFINDER_H


#include "CourtCombinationFinder.h"

class BruteForceCombinationFinder : public CourtCombinationFinder {
public:
    using CourtCombinationFinder::CourtCombinationFinder;

protected:
    std::vector<CourtAllocation> doFind(nonstd::span<const PlayerInfo> span, size_t numCourtAvailable) const override;
};


#endif //GAMEMATCHER_BRUTEFORCECOMBINATIONFINDER_H
