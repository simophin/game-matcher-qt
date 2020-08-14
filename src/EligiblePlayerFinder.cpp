//
// Created by Fanchao Liu on 14/08/20.
//

#include "EligiblePlayerFinder.h"
#include "GameStats.h"

#include <random>
#include <algorithm>

QVector<PlayerInfo>
EligiblePlayerFinder::findEligiblePlayers(
        const QVector<Member> &members,
        size_t playerPerCourt,
        size_t numCourt,
        const GameStats &stats,
        int randomSeed) {
    QVector<PlayerInfo> players;

    if (members.empty() || playerPerCourt == 0 || numCourt == 0) return players;

    if (stats.numGames() == 0) {
        players.reserve(members.size());
        // First game everyone is eligible
        for (const auto &m : members) {
            players.push_back(PlayerInfo{m, false});
        }
        return players;
    }

    // The number of people that will be on the court. Will be less or equal than number of members
    const size_t numMembersOn = members.size() / playerPerCourt * numCourt;


    struct MemberInfo {
        const Member *member;
        int eligibilityScore;
    };

    QVector<MemberInfo> memberInfo;
    memberInfo.reserve(members.size());
    for (const auto &member : members) {
        memberInfo.append(MemberInfo{&member, stats.numGamesOff(member.id) * 2000 - stats.numGamesFor(member.id)});
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(memberInfo.begin(), memberInfo.end(), std::default_random_engine(randomSeed));

    // Sort by number of games off
    std::stable_sort(memberInfo.begin(), memberInfo.end(), [](MemberInfo &a, MemberInfo &b) {
        return b.eligibilityScore < a.eligibilityScore;
    });

    if (memberInfo.size() > numMembersOn) {
        // If we don't have enough seats for all players, we will have two groups of people
        //  1. Ones that must be on
        //  2. Ones that are optionally on.
        // The ones that must on have higher numGamesOff than then lowest one

        const auto lowestScore = memberInfo[numMembersOn - 1].eligibilityScore;

        // Anything lower than lowestNumGamesOff will be discarded
        while (memberInfo.rbegin()->eligibilityScore < lowestScore) {
            memberInfo.pop_back();
        }

        for (const auto &info : memberInfo) {
            // If we have higher than lowestNumGamesOff, they are people must on.
            players.append(PlayerInfo{*info.member, info.eligibilityScore > lowestScore});
        }
    } else {
        // All members will be on the court.
        for (const auto &info : memberInfo) {
            players.append(PlayerInfo{*info.member, false});
        }
    }

    return players;
}