//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_MATCHINGSCORE_H
#define GAMEMATCHER_MATCHINGSCORE_H

#include "span.h"

class GameStats;

struct PlayerInfo;

class MatchingScore {
public:

    static int levelRangeScore(nonstd::span<const PlayerInfo> members, int minLevel, int maxLevel);

    static int levelStdVarianceScore(nonstd::span<const PlayerInfo> members);

    static int genderSimilarityScore(nonstd::span<const PlayerInfo> players);

    static int computeCourtScore(const GameStats &stats,
                                 nonstd::span<const PlayerInfo> players, int minLevel, int maxLevel);

};

#endif //GAMEMATCHER_MATCHINGSCORE_H
