#include "gamematcher.h"

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <list>
#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>

#include "gamestats.h"


struct PlayerInfo {
    Player player;
    Member member;
    int eligibilityScore = 0;
    int matchingScore = 0;
};

template <typename T>
static void sortByEligibilityScoreReverse(T &list) {
    std::sort(list.begin(), list.end(), [](const auto &a, const auto &b) {
        return a.eligibilityScore > b.eligibilityScore;
    });
}

template <typename T>
static void sortByMatchingScore(T &list) {
    std::sort(list.begin(), list.end(), [](const auto &a, const auto &b) {
        return a->matchingScore < b->matchingScore;
    });
}

static std::list<PlayerInfo> findEligiblePlayers(const QVector<Member> &members,
                                                 const QVector<Player> &players,
                                                 const GameStats &stats, int randomSeed) {
    auto memberById = std::reduce(
            members.constBegin(), members.constEnd(), QHash<MemberId, Member>(),
            [](auto &map, const auto &member) {
                map[member.id] = member;
                return map;
            });

    std::vector<PlayerInfo> eligiblePlayers;
    for (const auto &player : players) {
        if (player.paused || player.checkOutTime.isValid()) continue;
        const auto member = memberById.find(player.memberId);
        if (member == memberById.constEnd()) continue;

        auto &p = eligiblePlayers.emplace_back();
        p.player = player;
        p.member = *member;
        p.eligibilityScore = -stats.numGamesFor(player.id);
        if (stats.numGames() == 0) {
            p.eligibilityScore += member->level;
        }
    }

    std::shuffle(eligiblePlayers.begin(), eligiblePlayers.end(), std::default_random_engine(randomSeed));

    sortByEligibilityScoreReverse(eligiblePlayers);

    return std::list<PlayerInfo>(eligiblePlayers.cbegin(), eligiblePlayers.cend());
}

QVector<GameAllocation> GameMatcher::match(const QVector<GameAllocation> &pastAllocation,
                         const QVector<Member> &members,
                         const QVector<Player> &players,
                         const QVector<CourtId> &courts,
                         int playerPerCourt,
                         int seed) const {
    GameStats stats(pastAllocation, members, players);

    // Find eligible players
    auto eligiblePlayers = findEligiblePlayers(members, players, stats, seed);

    // Cut eligible players list
    const auto numPlayers = std::min<int>(eligiblePlayers.size() / playerPerCourt * playerPerCourt, playerPerCourt * courts.size());
    {
        auto iter = eligiblePlayers.begin();
        for (int i = 0; i < numPlayers - 1; i++) ++iter;
        const auto lowestEligibleScore = iter->eligibilityScore;
        for (; iter != eligiblePlayers.cend(); ++iter) {
            if (lowestEligibleScore != iter->eligibilityScore) {
                iter = eligiblePlayers.erase(iter);
            }
        }
    }

    std::vector<decltype(eligiblePlayers)::iterator> opponents;

    QVector<GameAllocation> result;
    result.reserve(numPlayers);
    auto court = courts.cbegin();

    while (eligiblePlayers.size() >= playerPerCourt) {
        auto iter = eligiblePlayers.begin();
        auto matchee = iter++;

        // Add rest of the eligible players to opponent list
        opponents.clear();
        opponents.reserve(eligiblePlayers.size() - 1);
        for (; iter != eligiblePlayers.end(); iter++) {
            iter->matchingScore = -stats.numGamesBetween(matchee->player.id, iter->player.id) +
                                  std::abs(matchee->member.level - iter->member.level) * 10;
            opponents.push_back(iter);
        }

        // Sort opponent list
        sortByMatchingScore(opponents);

        GameAllocation ga;

        // Push matchee itself
        ga.courtId = *court;
        ga.playerId = matchee->player.id;
        result.push_back(ga);
        eligiblePlayers.erase(matchee);

        // Push other opponents
        for (int i = 0; i < playerPerCourt - 1; i++) {
            auto &opponentIter = *opponents.rbegin();
            ga.playerId = opponentIter->player.id;
            eligiblePlayers.erase(opponentIter);
            opponents.pop_back();
            result.push_back(ga);
        }
        court++;
    }

    return result;
}

