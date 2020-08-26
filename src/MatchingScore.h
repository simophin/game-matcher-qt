//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_MATCHINGSCORE_H
#define GAMEMATCHER_MATCHINGSCORE_H

#include <cmath>
#include "models.h"

#include "GameStats.h"
#include "CollectionUtils.h"

class MatchingScore {
public:

    template <typename PlayerInfoList>
    static int levelRangeScore(const PlayerInfoList &members, int minLevel, int maxLevel) {
        if (members.size() < 2) return 0;

        int min = maxLevel + 1, max = minLevel - 1;
        for (const auto &m : members) {
            auto level = m->level;
            if (level > max) max = level;
            if (level < min) min = level;
        }

        return (max - min) * 100 / (maxLevel - minLevel);
    }

    template <typename PlayerInfoList>
    static int levelStdVarianceScore(const PlayerInfoList &members, int minLevel, int maxLevel) {
        if (members.size() < 2) return 0;

        const int sum = reduceCollection(members, 0, [](auto sum, const auto &info) {
            return sum + info->level;
        });
        const double average = static_cast<double>(sum) / members.size();

        return std::sqrt(reduceCollection(members, 0.0, [average](auto sum, const auto &info) {
            auto diff = info->level - average;
            return sum + diff * diff;
        }) / (members.size() - 1)) * 100 / (maxLevel - minLevel);
    }

    template <typename PlayerInfoList>
    static int genderSimilarityScore(const PlayerInfoList &players) {
        if (players.size() < 2) return 100;

        int numMales = std::count_if(players.begin(), players.end(), [](const auto &p) {
            return p->gender == Member::Male;
        });

        int numFemales = players.size() - numMales;
        if (numMales == 0 || numMales == players.size() || numFemales == numMales) return 100;

        int maxDiff = players.size() - 2;
        return (maxDiff - std::abs(numMales - numFemales)) * 100 / maxDiff;
    }

    template <typename PlayerInfoList>
    static int computeCourtScore(const GameStats &stats, const PlayerInfoList &players, int minLevel, int maxLevel) {
        thread_local QVector<MemberId> playerIds;

        playerIds.clear();
        for (const auto &p : players) {
            playerIds.push_back(p->memberId);
        }

        auto simScore = -stats.similarityScore(playerIds) * 3;
        auto varianceScore = -levelStdVarianceScore(players, minLevel, maxLevel) * 2;
        auto levelScore = -levelRangeScore(players, minLevel, maxLevel) * 2;
        auto genderScore = genderSimilarityScore(players);
        int score = simScore + varianceScore + levelScore + genderScore;
        return score;
    }

};

#endif //GAMEMATCHER_MATCHINGSCORE_H
