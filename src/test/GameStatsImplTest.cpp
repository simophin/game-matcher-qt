//
// Created by Fanchao Liu on 20/08/20.
//

#include <catch2/catch.hpp>

#include "GameStats.h"

#include <map>
#include <QVector>

TEST_CASE("GameStatsImplTest") {
    auto[input, numGames, numGamesOff, totalGame] = GENERATE(
            table<QVector<GameAllocation>, std::map<MemberId, int>, std::map<MemberId, int>, int>(
                    {
                            {
                                    {
                                            GameAllocation(0, 0, 1, 1),
                                            GameAllocation(0, 0, 2, 1),
                                    },
                                    {
                                            {1, 1},
                                            {2, 1},
                                            {3, 0},
                                    },
                                    {
                                            {1, 0},
                                            {2, 0},
                                            {3, 1},
                                    },
                                    1,
                            },
                            {
                                    {
                                            GameAllocation(0, 0, 1, 1),
                                            GameAllocation(0, 0, 2, 1),
                                            GameAllocation(1, 1, 3, 1),
                                            GameAllocation(1, 1, 2, 1),
                                    },
                                    {
                                            {1, 1},
                                            {2, 2},
                                            {3, 1},
                                    },
                                    {
                                            {1, 1},
                                            {2, 0},
                                            {3, 0},
                                    },
                                    2,
                            },
                    }
            ));

    GameStatsImpl stats(input);
    for (auto[member, expected] : numGames) {
        REQUIRE(stats.numGamesFor(member) == expected);
    }
    for (auto[member, expected] : numGamesOff) {
        REQUIRE(stats.numGamesOff(member) == expected);
    }
    REQUIRE(stats.numGames() == totalGame);
}