//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_COURTCOMBINATIONFINDER_H
#define GAMEMATCHER_COURTCOMBINATIONFINDER_H

#include "models.h"

#include <QVector>

class GameStats;
struct PlayerInfo;

class CourtCombinationFinder {
public:
    CourtCombinationFinder(const GameStats &stats, unsigned numPlayersPerCourt)
            : stats_(stats), numPlayersPerCourt_(numPlayersPerCourt) {}

    QVector<GameAllocation> find(const QVector<CourtId> &courts, const QVector<PlayerInfo> &players);

protected:
    struct CourtAllocation {
        QVector<PlayerInfo> players;
        int quality;
    };
    virtual QVector<CourtAllocation> doFind(const QVector<PlayerInfo> &, unsigned numCourtAvailable) const = 0;

    GameStats const &stats_;
    unsigned const numPlayersPerCourt_;
};


#endif //GAMEMATCHER_COURTCOMBINATIONFINDER_H
