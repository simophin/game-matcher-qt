//
// Created by Fanchao Liu on 15/08/20.
//

#ifndef GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H
#define GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H

#include "CombinationFinder.h"
#include "PlayerInfo.h"


enum class Sorting {
    ASC, DESC,
};

class SortingLevelCombinationFinder : public CombinationFinder {
public:
    SortingLevelCombinationFinder(unsigned int numPlayersPerCourt,
                                  int randomSeed,
                                  Sorting sorting = Sorting::DESC)
            : CombinationFinder(numPlayersPerCourt), randomSeed_(randomSeed), sorting_(sorting) {}

protected:
    QVector<CourtAllocation> doFind(const QVector<PlayerInfo> &players, unsigned int numCourtAvailable) const override;

private:
    int const randomSeed_;
    Sorting const sorting_;
};


#endif //GAMEMATCHER_SORTINGLEVELCOMBINATIONFINDER_H
