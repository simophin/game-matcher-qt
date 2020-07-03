#include "GameMatcher.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <list>
#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>

#include "GameStats.h"
#include "ClubRepository.h"


struct PlayerInfo {
    MemberInfo member;
    int eligibilityScore = 0;
    int matchingScore = 0;
};

template<typename T>
static void sortByEligibilityScoreReverse(T &list) {
    std::sort(list.begin(), list.end(), [](const auto &a, const auto &b) {
        return a.eligibilityScore > b.eligibilityScore;
    });
}

template<typename T>
static void sortByMatchingScore(T &list) {
    std::sort(list.begin(), list.end(), [](const auto &a, const auto &b) {
        return a->matchingScore < b->matchingScore;
    });
}

static std::list<PlayerInfo> getEligiblePlayers(const QVector<MemberInfo> &members,
                                                int numSeats,
                                                const GameStats &stats, int randomSeed) {
    std::vector<PlayerInfo> players;
    for (const auto &member : members) {
        auto &p = players.emplace_back();
        p.member = member;
        p.eligibilityScore = member.numGames < 0 ? 0 : (-member.numGames);
        if (stats.numGames() == 0) {
            p.eligibilityScore += member.level;
        }
    }

    std::shuffle(players.begin(), players.end(), std::default_random_engine(randomSeed));
    sortByEligibilityScoreReverse(players);

    if (players.size() > numSeats && numSeats > 0) {
        // Find lowest score and we will cut off from there
        int cutOffStart = numSeats;
        const auto lowestScore = players[numSeats - 1].eligibilityScore;
        for (; cutOffStart < players.size(); cutOffStart++) {
            if (players[cutOffStart].eligibilityScore != lowestScore) break;
        }
        return std::list<PlayerInfo>(players.cbegin(), players.cbegin() + cutOffStart);
    }

    return std::list<PlayerInfo>(players.cbegin(), players.cend());
}

QFuture<QVector<GameAllocation>> GameMatcher::match(
        const QVector<GameAllocation> &pastAllocation,
        const QVector<MemberInfo> &members,
        const QVector<CourtId> &courts,
        int playerPerCourt,
        int seed) {
    return QtConcurrent::run([=] {
        qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size() << " players and "
                 << courts.size() << " courts";
        GameStats stats(pastAllocation, members);

        // Find eligible players
        auto eligiblePlayers = getEligiblePlayers(members, playerPerCourt * courts.size(), stats, seed);

        std::vector<decltype(eligiblePlayers)::iterator> opponents;

        QVector<GameAllocation> result;
        auto court = courts.cbegin();

        while (eligiblePlayers.size() >= playerPerCourt) {
            auto iter = eligiblePlayers.begin();
            auto matchee = iter++;

            // Add rest of the eligible players to opponent list
            opponents.clear();
            opponents.reserve(eligiblePlayers.size() - 1);
            for (; iter != eligiblePlayers.end(); iter++) {
                iter->matchingScore = -stats.numGamesBetween(matchee->member.id, iter->member.id) +
                                      std::abs(matchee->member.level - iter->member.level) * 10;
                opponents.push_back(iter);
            }

            // Sort opponent list
            sortByMatchingScore(opponents);

            GameAllocation ga;

            // Push matchee itself
            ga.courtId = *court;
            ga.memberId = matchee->member.id;
            result.push_back(ga);
            eligiblePlayers.erase(matchee);

            // Push other opponents
            for (int i = 0; i < playerPerCourt - 1; i++) {
                auto &opponentIter = *opponents.rbegin();
                ga.memberId = opponentIter->member.id;
                eligiblePlayers.erase(opponentIter);
                opponents.pop_back();
                result.push_back(ga);
            }
            court++;
        }

        qDebug() << "Matched result has " << result.size() << " allocations";

        return result;
    });

}

