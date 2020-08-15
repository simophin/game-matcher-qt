//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_BFCOMBINATIONFINDER_H
#define GAMEMATCHER_BFCOMBINATIONFINDER_H


#include "CombinationFinder.h"

class GameStats;

class BFCombinationFinder : public CombinationFinder {
public:

    BFCombinationFinder(unsigned int numPlayersPerCourt, const GameStats &stats)
            : CombinationFinder(numPlayersPerCourt), stats_(stats) {}

protected:
    QVector<CourtAllocation> doFind(const QVector<PlayerInfo> &,
                                    unsigned numCourtAvailable) const override;

private:
    GameStats const &stats_;
};


#endif //GAMEMATCHER_BFCOMBINATIONFINDER_H
