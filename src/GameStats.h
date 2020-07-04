//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMESTATS_H
#define GAMEMATCHER_GAMESTATS_H

#include <QHash>
#include <QVector>
#include <QThreadStorage>

#include <set>
#include <algorithm>

#include "models.h"
#include "ClubRepository.h"

struct PlayerInfo {
    MemberInfo member;
    std::optional<int> eligibilityScore;

    PlayerInfo(const MemberInfo &member, int eligibilityScore)
            : member(member), eligibilityScore(eligibilityScore) {}
};

class GameStats {
    QVector<QVector<MemberId>> courtPlayers;
    int numTotalGames = 0;

public:
    GameStats(const QVector<GameAllocation> &pastAllocation) {
        QHash<GameId, QHash<CourtId, QVector<MemberId>>> map;
        for (const auto &allocation : pastAllocation) {
            map[allocation.gameId][allocation.courtId].append(allocation.memberId);
        }

        for (const auto &game : map) {
            for (const auto &court : game) {
                courtPlayers.append(court);
                std::sort(courtPlayers.rbegin()->begin(), courtPlayers.rbegin()->end());
            }
        }

        numTotalGames = map.size();
    }

    int numGames() const { return this->numTotalGames; }

    int similarityWithPast(const QVector<std::list<PlayerInfo>::const_iterator> &players) const {
        QThreadStorage<QVector<MemberId>> sortedPlayers;
        auto &sorted = sortedPlayers.localData();
        sorted.clear();
        for (auto p : players) sorted.push_back(p->member.id);
        std::sort(sorted.begin(), sorted.end());

        int sum = 0;
        for (const auto &court : courtPlayers) {
            for (auto i = 0, size = std::min(court.size(), players.size()); i < size; i++) {
                if (court[i] == players[i]->member.id) sum++;
                else break;
            }
        }
        return sum;
    }
};

#endif //GAMEMATCHER_GAMESTATS_H
