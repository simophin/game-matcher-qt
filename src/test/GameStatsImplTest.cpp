//
// Created by Fanchao Liu on 20/08/20.
//

#include <catch2/catch.hpp>

#include "GameStats.h"
#include "TupleVector.h"

#include <map>
#include <QVector>

TEST_CASE("GameStatsImplTest") {
    auto[input, numGames, numGamesOff, totalGame, score] = GENERATE(
            table<QVector<GameAllocation>, TupleVector<MemberId, int>, TupleVector<MemberId, int>, int, TupleVector<QVector<MemberId>, int>>(
                    {
                            {
                                    {
                                            GameAllocation(0, 0, 1, 1),
                                            GameAllocation(0, 0, 2, 1),
                                    },
                                    {
                                            {1, 1}, {2, 1}, {3, 0},
                                    },
                                    {
                                            {1, 0}, {2, 0}, {3, 1},
                                    },
                                    1,
                                    {
                                            {{1, 2}, 100}, {{1, 3}, 0}, {{2, 3}, 0},
                                    },
                            },
                            {
                                    {
                                            GameAllocation(0, 0, 1, 1),
                                            GameAllocation(0, 0, 2, 1),
                                            GameAllocation(0, 0, 3, 1),
                                            GameAllocation(0, 0, 4, 1),
                                            GameAllocation(1, 1, 3, 1),
                                            GameAllocation(1, 1, 4, 1),
                                            GameAllocation(1, 1, 5, 1),
                                            GameAllocation(1, 1, 6, 1),
                                            GameAllocation(2, 1, 5, 1),
                                            GameAllocation(2, 1, 6, 1),
                                            GameAllocation(2, 1, 1, 1),
                                            GameAllocation(2, 1, 2, 1),
                                    },
                                    {
                                            {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {7, 0},
                                    },
                                    {
                                            {1, 0}, {2, 0}, {3, 1}, {4, 1}, {5, 0}, {6, 0}, {7, 3},
                                    },
                                    3,
                                    {
                                            {{1, 2, 3, 4}, 66},
                                            {{1, 2, 3, 5}, 66},
                                            {{2, 3, 6, 7}, 50},
                                            {{2, 3, 5, 7}, 50},
                                            {{7, 8, 9, 10}, 0},
                                    },
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
    for (auto [members, expected] : score) {
        REQUIRE(stats.similarityScore(members) == expected);
    }
    REQUIRE(stats.numGames() == totalGame);
}