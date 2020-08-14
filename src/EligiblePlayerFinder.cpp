//
// Created by Fanchao Liu on 14/08/20.
//

#include "EligiblePlayerFinder.h"
#include "GameStats.h"

#include <random>

std::vector<PlayerInfo>
EligiblePlayerFinder::getEligiblePlayers(nonstd::span<const Member> members, int numMaxSeats, const GameStats &stats,
                                         int randomSeed) {
    struct MemberInfo {
        const Member *member;
        int eligibilityScore;
    };

    std::vector<PlayerInfo> players;

    if (members.empty() || numMaxSeats <= 0) return players;

    std::vector<MemberInfo> memberInfo;
    memberInfo.reserve(members.size());
    for (const auto &member : members) {
        memberInfo.push_back({&member, stats.numGamesOff(member.id) * 2000 - stats.numGamesFor(member.id)});
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(memberInfo.begin(), memberInfo.end(), std::default_random_engine(randomSeed));

    // Sort by number of games off
    std::sort(memberInfo.begin(), memberInfo.end(), [](MemberInfo &a, MemberInfo &b) {
        return b.eligibilityScore < a.eligibilityScore;
    });

    if (memberInfo.size() > numMaxSeats) {
        // If we don't have enough seats for all players, we will have two groups of people
        //  1. Ones that must be on
        //  2. Ones that are optionally on.
        // The ones that must on have higher numGamesOff than then lowest one

        const auto lowestScore = memberInfo[numMaxSeats - 1].eligibilityScore;

        // Anything lower than lowestNumGamesOff will be discarded
        while (memberInfo.rbegin()->eligibilityScore < lowestScore) {
            memberInfo.pop_back();
        }

        for (const auto &info : memberInfo) {
            // If we have higher than lowestNumGamesOff, they are people must on.
            players.emplace_back(*info.member, info.eligibilityScore > lowestScore);
        }
    } else {
        for (const auto &info : memberInfo) {
            players.emplace_back(*info.member, true);
        }
    }

    return players;
}
