//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMESTATS_H
#define GAMEMATCHER_GAMESTATS_H

#include <map>
#include <QSet>

#include "models.h"
#include "span.h"
#include "PlayerInfo.h"


class GameStats {
    std::map<GameId, std::map<CourtId, QSet<MemberId>>> games;
    int numTotalGames = 0;

public:
    explicit GameStats(nonstd::span<const GameAllocation> pastAllocation) {
        for (const auto &allocation : pastAllocation) {
            games[allocation.gameId][allocation.courtId].insert(allocation.memberId);
        }

        numTotalGames = games.size();
    }


    int numGamesOff(MemberId memberId) const {
        int i = 0;
        for (auto iter = games.rbegin(); iter != games.rend(); ++iter) {
            for (const auto &court : iter->second) {
                if (court.second.contains(memberId)) {
                    return i;
                }
            }
            i++;
        }

        return i;
    }

    int numGames() const { return this->numTotalGames; }

    template<typename PlayerInfoList>
    int similarityScore(const PlayerInfoList &players) const {
        if (games.empty()) return 0;

        int totalSeats = 0;
        int sum = 0;
        for (const auto &[gameId, game] : games) {
            for (const auto &[courtId, court] : game) {
                int numPlayedHere = 0;
                for (const auto &p : players) {
                    if (court.contains(p->memberId)) {
                        numPlayedHere++;
                    }
                }

                if (numPlayedHere >= 2) {
                    totalSeats += std::min(static_cast<size_t>(court.size()), players.size());
                    sum += numPlayedHere;
                }
            }
        }

        if (totalSeats == 0) return 0;
        return sum * 100 / totalSeats;
    }
};

#endif //GAMEMATCHER_GAMESTATS_H
