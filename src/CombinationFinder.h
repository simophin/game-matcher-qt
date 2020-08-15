//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_COMBINATIONFINDER_H
#define GAMEMATCHER_COMBINATIONFINDER_H

#include "models.h"
#include "PlayerInfo.h"

#include <QVector>


class CombinationFinder {
public:
    explicit CombinationFinder(unsigned numPlayersPerCourt)
            : numPlayersPerCourt_(numPlayersPerCourt) {}

    virtual ~CombinationFinder() = default;

    QVector<GameAllocation> find(const QVector<CourtId> &courts, const QVector<PlayerInfo> &players);

protected:
    struct CourtAllocation {
        QVector<PlayerInfo> players;
        int quality;
    };

    virtual QVector<CourtAllocation> doFind(const QVector<PlayerInfo> &, unsigned numCourtAvailable) const = 0;

    unsigned const numPlayersPerCourt_;
};


#endif //GAMEMATCHER_COMBINATIONFINDER_H
