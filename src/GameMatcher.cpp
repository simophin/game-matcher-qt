#include "GameMatcher.h"

#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>
#include <random>

#include "GameStats.h"
#include "CombinationsFinder.h"
#include "FastVector.h"
#include "CourtFinderV1.h"

using nonstd::span;

static std::vector<PlayerInfo> getEligiblePlayers(
        span<const Member> members, int numMaxSeats, const GameStats &stats, int randomSeed) {
    struct MemberInfo {
        const Member *member;
        int numGamesOff;
    };

    std::vector<PlayerInfo> players;

    if (members.empty() || numMaxSeats <= 0) return players;

    std::vector<MemberInfo> memberInfo;
    memberInfo.reserve(members.size());
    for (const auto &member : members) {
        memberInfo.push_back({&member, stats.numGamesOff(member.id)});
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(memberInfo.begin(), memberInfo.end(), std::default_random_engine(randomSeed));

    // Sort by number of games off
    std::sort(memberInfo.begin(), memberInfo.end(), [](MemberInfo &a, MemberInfo &b) {
        return b.numGamesOff < a.numGamesOff;
    });

    if (memberInfo.size() > numMaxSeats) {
        // If we don't have enough seats for all players, we will have two groups of people
        //  1. Ones that must be on
        //  2. Ones that are optionally on.
        // The ones that must on have higher numGamesOff than then lowest one

        const auto lowestNumGamesOff = memberInfo[numMaxSeats - 1].numGamesOff;

        // Anything lower than lowestNumGamesOff will be discarded
        while (memberInfo.rbegin()->numGamesOff < lowestNumGamesOff) {
            memberInfo.pop_back();
        }

        for (const auto &info : memberInfo) {
            // If we have higher than lowestNumGamesOff, they are people must on.
            players.emplace_back(*info.member, info.numGamesOff > lowestNumGamesOff);
        }
    }

    return players;
}


std::vector<GameAllocation> GameMatcher::match(
        span<const GameAllocation> pastAllocation,
        span<const Member> members,
        span<const CourtId> courts,
        size_t playerPerCourt,
        unsigned int levelMin,
        unsigned int levelMax,
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

    return CourtFinderV1(stats, playerPerCourt, levelMin, levelMax).find(courts, eligiblePlayers);
}

