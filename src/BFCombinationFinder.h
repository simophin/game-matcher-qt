//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_BFCOMBINATIONFINDER_H
#define GAMEMATCHER_BFCOMBINATIONFINDER_H


#include "CourtCombinationFinder.h"

class BFCombinationFinder : public CourtCombinationFinder {
public:
    using CourtCombinationFinder::CourtCombinationFinder;

protected:
    std::vector<CourtAllocation> doFind(nonstd::span<const PlayerInfo> span, size_t numCourtAvailable) const override;
};


#endif //GAMEMATCHER_BFCOMBINATIONFINDER_H
