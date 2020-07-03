//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMESTATS_H
#define GAMEMATCHER_GAMESTATS_H

#include <QHash>
#include <QVector>
#include <QSet>
#include <algorithm>

#include "models.h"

class GameStats {
    struct GamePair {
        MemberId players[2];

        inline GamePair(MemberId a, MemberId b): players {a < b ? a : b, a < b ? b : a} {}

        inline bool operator==(const GamePair &rhs) const {
            return players == rhs.players;
        }
    };

    friend int qHash(GamePair);

    // Source of truth
    QVector<QSet<MemberId>> courtPlayers;
    int numTotalGames = 0;

    // Cache values
    mutable QHash<GamePair, int> numGamesByPair;

public:
    GameStats(const QVector<GameAllocation> &pastAllocation,
              const QVector<MemberInfo> &members) {
        QHash<GameId, QHash<CourtId, QSet<MemberId>>> map;
        for (const auto &allocation : pastAllocation) {
            map[allocation.gameId][allocation.courtId].insert(allocation.memberId);
        }

        for (const auto &game : map) {
            for (const auto &court : game) {
                courtPlayers.append(court);
            }
        }

        numTotalGames = map.size();
    }

    int numGames() const { return this->numTotalGames; }

    int numGamesBetween(MemberId a, MemberId b) const {
        GamePair pair(a, b);
        const auto found = numGamesByPair.find(pair);
        if (found != numGamesByPair.cend()) {
            return *found;
        }

        return numGamesByPair[pair] = std::reduce(
                courtPlayers.constBegin(), courtPlayers.constEnd(), 0,
                [a, b] (int sum, const auto &court) {
                    return (court.contains(a) && court.contains(b)) ? (sum + 1) : sum;
                });
    }
};

inline int qHash(GameStats::GamePair pair) {
    return qHash(pair.players);
}


#endif //GAMEMATCHER_GAMESTATS_H
