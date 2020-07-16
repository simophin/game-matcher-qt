//
// Created by Fanchao Liu on 16/07/20.
//

#include "MatchingScore.h"
#include "CollectionUtils.h"
#include "GameStats.h"
#include "PlayerInfo.h"

int MatchingScore::levelRangeScore(nonstd::span<const PlayerInfo> members, int minLevel, int maxLevel) {
    if (members.size() < 2) return 0;

    int min = maxLevel + 1, max = minLevel - 1;
    for (const PlayerInfo &m : members) {
        auto level = m.level;
        if (level > max) max = level;
        if (level < min) min = level;
    }

    return (max - min) * 100 / (maxLevel - minLevel);
}

int MatchingScore::levelStdVarianceScore(nonstd::span<const PlayerInfo> members) {
    if (members.size() < 2) return 0;

    const int sum = reduceCollection(members, 0, [](auto sum, const PlayerInfo &info) {
        return sum + info.level;
    });
    const double average = static_cast<double>(sum) / members.size();

    return std::sqrt(reduceCollection(members, 0.0, [average](auto sum, const PlayerInfo &info) {
        auto diff = info.level - average;
        return sum + diff * diff;
    }) / (members.size() - 1)) * 100 / (levelMax - levelMin);
}

int MatchingScore::genderSimilarityScore(nonstd::span<const PlayerInfo> players) {
    if (players.size() < 2) return 100;

    int numMales = std::count_if(players.begin(), players.end(), [](const PlayerInfo &p) {
        return p.gender == Member::Male;
    });

    int numFemales = players.size() - numMales;
    if (numMales == 0 || numMales == players.size() || numFemales == numMales) return 100;

    int maxDiff = players.size() - 2;
    return (maxDiff - std::abs(numMales - numFemales)) * 100 / maxDiff;
}

int
MatchingScore::computeCourtScore(const GameStats &stats, nonstd::span<const PlayerInfo> players, int minLevel, int maxLevel) {
    auto simScore = -stats.similarityScore(players) * 2;
    auto varianceScore = -levelStdVarianceScore(players) * 2;
    auto levelScore = -levelRangeScore(players, minLevel, maxLevel) * 8;
    auto genderScore = genderSimilarityScore(players);
    int score = simScore + varianceScore + levelScore + genderScore;
    qDebug().noquote().nospace() << "Evaluating court " << players
                                 << ",similarityScore=" << simScore
                                 << ",varianceScore=" << varianceScore
                                 << ",levelScore=" << levelScore
                                 << ",genderScore=" << genderScore
                                 << ",totalScore = " << score;
    return score;
}
