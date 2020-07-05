//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMEMATCHERINTERNAL_H
#define GAMEMATCHER_GAMEMATCHERINTERNAL_H

#include <QHash>
#include <QVector>
#include <QSet>
#include <QThreadStorage>

#include <list>
#include <algorithm>
#include <map>

#include "models.h"
#include "ClubRepository.h"
#include "span.h"

struct PlayerInfo {
    Member member;
    std::optional<int> eligibilityScore;

    PlayerInfo(const Member &member, int eligibilityScore)
            : member(member), eligibilityScore(eligibilityScore) {}
};

typedef std::list<PlayerInfo> PlayerList;
typedef typename PlayerList::const_iterator PlayerListIterator;

class GameStats {
    QVector<QSet<MemberId>> courtPlayers;
    int numTotalGames = 0;

public:
    GameStats(const QVector<GameAllocation> &pastAllocation) {
        std::map<GameId, QHash<CourtId, QSet<MemberId>>> map;
        for (const auto &allocation : pastAllocation) {
            map[allocation.gameId][allocation.courtId].insert(allocation.memberId);
        }

        for (const auto &[id, game] : map) {
            for (const auto &court : game) {
                courtPlayers.append(court);
            }
        }

        numTotalGames = map.size();
    }


    int numGamesOff(MemberId memberId) const {
        int i = 0;
        for (auto iter = courtPlayers.crbegin(); iter != courtPlayers.crend(); iter++, i++) {
            if (iter->contains(memberId)) break;
        }
        return i;
    }

    int numGames() const { return this->numTotalGames; }

    int similarityScore(nonstd::span<PlayerListIterator> players) const {
        if (courtPlayers.isEmpty()) return 0;

        int totalSeats = 0;
        int sum = 0;
        for (const auto &court : courtPlayers) {
            int numPlayedHere = 0;
            for (const auto &p : players) {
                if (court.contains(p->member.id)) {
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

#endif //GAMEMATCHER_GAMEMATCHERINTERNAL_H
