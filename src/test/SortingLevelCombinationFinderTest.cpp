//
// Created by Fanchao Liu on 20/08/20.
//

#include <catch2/catch.hpp>
#include "SortingLevelCombinationFinder.h"
#include "models.h"

auto playerInfo(MemberId id, int level) {
    return PlayerInfo(id, Member::Gender::Male, level, false);
}

TEST_CASE("SortingLevelCombinationFinder") {
    registerModels();

    auto[players, courts, sorting, numPerCourt, expect] = GENERATE(
            table<QVector<PlayerInfo>, QVector<CourtId>, Sorting, int, QVector<GameAllocation>>(
                    {
                            {
                                    {
                                            playerInfo(1, 1),
                                            playerInfo(2, 2),
                                            playerInfo(3, 3),
                                            playerInfo(4, 4),
                                    },
                                    {1, 2},
                                    Sorting::ASC,
                                    2,
                                    {
                                            GameAllocation(0, 1, 1, 100),
                                            GameAllocation(0, 1, 2, 100),
                                            GameAllocation(0, 2, 3, 100),
                                            GameAllocation(0, 2, 4, 100),
                                    },
                            },
                            {
                                    {
                                            playerInfo(1, 4),
                                            playerInfo(2, 3),
                                            playerInfo(3, 2),
                                            playerInfo(4, 1),
                                    },
                                    {1, 2},
                                    Sorting::DESC,
                                    2,
                                    {
                                            GameAllocation(0, 1, 1, 100),
                                            GameAllocation(0, 1, 2, 100),
                                            GameAllocation(0, 2, 3, 100),
                                            GameAllocation(0, 2, 4, 100),
                                    },
                            },
                    }
            ));
    SortingLevelCombinationFinder finder(numPerCourt, 0, sorting);
    REQUIRE(finder.find(courts, players) == expect);
}