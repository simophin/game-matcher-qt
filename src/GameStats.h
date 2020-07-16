//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMESTATS_H
#define GAMEMATCHER_GAMESTATS_H

#include <set>
#include <map>

#include "models.h"
#include "span.h"
#include "PlayerInfo.h"


class GameStats {
    std::vector<std::set<MemberId>> courtPlayers;
    int numTotalGames = 0;

public:
    explicit GameStats(nonstd::span<const GameAllocation> pastAllocation) {
        std::map<GameId, std::map<CourtId, std::set<MemberId>>> map;
        for (const auto &allocation : pastAllocation) {
            map[allocation.gameId][allocation.courtId].insert(allocation.memberId);
        }

        numTotalGames = map.size();

        for (auto &[gameId, game] : map) {
            for (auto &[courtId, players] : game) {
                courtPlayers.emplace_back(std::move(players));
            }
        }
    }


    int numGamesOff(MemberId memberId) const {
        int i = 0;
        for (auto iter = courtPlayers.crbegin(); iter != courtPlayers.crend(); iter++, i++) {
            if (iter->find(memberId) != iter->end()) break;
        }
        return i;
    }

    int numGames() const { return this->numTotalGames; }

    int similarityScore(nonstd::span<const PlayerInfo> players) const {
        if (courtPlayers.empty()) return 0;

        int totalSeats = 0;
        int sum = 0;
        for (const auto &court : courtPlayers) {
            int numPlayedHere = 0;
            for (const PlayerInfo &p : players) {
                if (court.find(p.memberId) != court.end()) {
                    numPlayedHere++;
                }
            }

            if (numPlayedHere >= 2) {
                totalSeats += std::min(static_cast<size_t>(court.size()), players.size());
                sum += numPlayedHere;
            }
        }

        if (totalSeats == 0) return 0;
        return sum * 100 / totalSeats;
    }
};

#endif //GAMEMATCHER_GAMESTATS_H
