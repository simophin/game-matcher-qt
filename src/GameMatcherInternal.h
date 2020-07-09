//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMEMATCHERINTERNAL_H
#define GAMEMATCHER_GAMEMATCHERINTERNAL_H

#include <QHash>
#include <QVector>
#include <QSet>
#include <QThreadStorage>
#include <QSharedData>

#include <list>
#include <algorithm>
#include <map>

#include "models.h"
#include "ClubRepository.h"
#include "span.h"

class PlayerInfo {
    class Data : public QSharedData {
    public:
        Member member;
        std::optional<int> eligibilityScore;

        inline Data(const Member &member, int eligibilityScore)
            : member(member), eligibilityScore(eligibilityScore) {}
    };

    QSharedDataPointer<Data> d;

public:
    inline auto id() const { return d->member.id; }
    inline auto level() const { return d->member.level; }
    inline auto gender() const { return d->member.gender; }
    inline auto eligibilityScore() const { return d->eligibilityScore; }
    inline void clearEligibilityScore() {
        d->eligibilityScore.reset();
    }

    PlayerInfo(const Member &member, int eligibilityScore)
            : d(new Data({member, eligibilityScore})){}
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

    template <typename Col>
    int similarityScore(const Col &players) const {
        if (courtPlayers.empty()) return 0;

        int totalSeats = 0;
        int sum = 0;
        for (const auto &court : courtPlayers) {
            int numPlayedHere = 0;
            for (const PlayerInfo &p : players) {
                if (court.contains(p.id())) {
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
