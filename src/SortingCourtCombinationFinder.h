//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_SORTINGCOURTCOMBINATIONFINDER_H
#define GAMEMATCHER_SORTINGCOURTCOMBINATIONFINDER_H

#include "CourtCombinationFinder.h"

class SortingCourtCombinationFinder : public CourtCombinationFinder {
public:
    using CourtCombinationFinder::CourtCombinationFinder;

protected:
    std::vector<int> doFind(nonstd::span<const PlayerInfo> span, size_t numCourtAvailable) const override;
};


#endif //GAMEMATCHER_SORTINGCOURTCOMBINATIONFINDER_H
