#include <catch2/catch.hpp>

#include "EligiblePlayerFinder.h"

TEST_CASE("EligiblePlayerFinder") {
    auto numCourt = GENERATE(1, 2, 3, 4);
//    auto playerPerCourt = GENERATE();

//    auto &[gameAllocations, members, playerPerCourt, numCourt] =
//    GENERATE(table<QVector<GameAllocation>, QVector<BasePlayerInfo>, size_t, size_t>(
//            {
//                    {
//                            {},
//                            {
//                                BasePlayerInfo(1, BaseMember::Male, 1),
//                                BasePlayerInfo(2, BaseMember::Female, 2),
//                                BasePlayerInfo(3, BaseMember::Female, 3),
//                                BasePlayerInfo(4, BaseMember::Female, 4),
//                            },
//                            4,
//                            4,
//                    },
//            }));
}