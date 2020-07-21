#include "GameMatcher.h"

#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>
#include <random>

#include "GameStats.h"
#include "CombinationsFinder.h"
#include "FastIntVector.h"
#include "BruteForceCombinationFinder.h"

using nonstd::span;

static std::vector<PlayerInfo> getEligiblePlayers(
        span<const Member> members, int numMaxSeats, const GameStats &stats, int randomSeed) {
    std::vector<PlayerInfo> players;
    if (members.empty()) return {};

    for (const auto &member : members) {
        players.emplace_back(member, stats.numGamesOff(member.id));
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(players.begin(), players.end(), std::default_random_engine(randomSeed));

    // Sort by eligibility score max to min
    std::sort(players.begin(), players.end(), [](auto &a, auto &b) {
        return b.eligibilityScore.value() < a.eligibilityScore.value();
    });

    if (players.size() > numMaxSeats && numMaxSeats > 0) {
        // Find lowest score and we will cut off from there
        int cutOffSize = numMaxSeats;
        const auto lowestScore = players[numMaxSeats - 1].eligibilityScore.value();
        for (; cutOffSize < players.size(); cutOffSize++) {
            if (players[cutOffSize].eligibilityScore.value() != lowestScore) break;
        }

        while (players.size() > cutOffSize) {
            players.pop_back();
        }

        if (players.begin()->eligibilityScore.value() != lowestScore) {
            // The lowest score players will be re-written to invalid to indicate they are all optional, if there
            // are higher score players.
            for (auto iter = players.rbegin();
                 iter != players.rend() && iter->eligibilityScore.value() == lowestScore;
                 ++iter) {
                iter->eligibilityScore = std::nullopt;
            }
        }
    }

    return players;
}


std::vector<GameAllocation> GameMatcher::match(
        span<const GameAllocation> pastAllocation,
        span<const Member> members,
        span<const CourtId> courts,
        size_t playerPerCourt,
        int seed) {
    qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size()
             << " players and "
             << courts.size() << " courts";

    std::vector<GameAllocation> result;

    const size_t numMaxSeats = std::min(playerPerCourt * courts.size(),
                                        members.size() / playerPerCourt * courts.size());
    if (pastAllocation.empty()) {
        std::vector<Member> sortedMembers(members.begin(), members.end());
        std::shuffle(sortedMembers.begin(), sortedMembers.end(), std::default_random_engine(seed));

        // First game, sort players by level and only take the top to down people
        std::sort(sortedMembers.begin(), sortedMembers.end(), [](const auto &a, const auto &b) {
            return b.level < a.level;
        });

        const auto numMembers = std::min(numMaxSeats, sortedMembers.size() / playerPerCourt * playerPerCourt);
        result.reserve(numMembers);
        auto courtIter = courts.begin();
        for (int i = 0; i < numMembers; i++) {
            result.emplace_back(0, *courtIter, sortedMembers[i].id, 100);

            // Advance to next court when this one is full
            if ((i + 1) % playerPerCourt == 0) {
                courtIter++;
            }
        }
        return result;
    }

    GameStats stats(pastAllocation);

    // Find eligible players
    auto eligiblePlayers = getEligiblePlayers(members, numMaxSeats, stats, seed);

    return BruteForceCombinationFinder(stats, playerPerCourt).find(courts, eligiblePlayers);
}

