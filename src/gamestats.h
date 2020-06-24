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
        PlayerId players[2];

        inline GamePair(PlayerId a, PlayerId b): players {a < b ? a : b, a < b ? b : a} {}

        inline bool operator==(const GamePair &rhs) const {
            return players == rhs.players;
        }
    };

    friend int qHash(GamePair);

    // Source of truth
    QVector<QSet<PlayerId>> courtPlayers;

    // Cache values
    mutable QHash<GamePair, int> numGamesByPair;
    mutable QHash<PlayerId, int> numGamesByPlayer;

public:
    GameStats(const QVector<GameAllocation> &pastAllocation,
              const QVector<Member> &members,
              const QVector<Player> &players) {
        QHash<GameId, QHash<CourtId, QSet<PlayerId>>> map;
        for (const auto &allocation : pastAllocation) {
            map[allocation.gameId][allocation.courtId].insert(allocation.playerId);
        }

        for (const auto &game : map) {
            for (const auto &court : game) {
                courtPlayers.append(court);
            }
        }
    }

    int numGamesBetween(PlayerId a, PlayerId b) const {
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

    int numGamesFor(PlayerId id) const {
        const auto found = numGamesByPlayer.find(id);
        if (found != numGamesByPlayer.cend()) {
            return *found;
        }

        return numGamesByPlayer[id] = std::reduce(
                courtPlayers.constBegin(), courtPlayers.constEnd(), 0,
                [id](int sum, const auto &court) {
                    return court.contains(id) ? (sum + 1) : sum;
                });
    }
};

inline int qHash(GameStats::GamePair pair) {
    return qHash(pair.players);
}


#endif //GAMEMATCHER_GAMESTATS_H
