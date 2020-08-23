//
// Created by Fanchao Liu on 21/08/20.
//

#include <catch2/catch.hpp>
#include "BFCombinationFinder.h"
#include "MockGameStats.h"

TEST_CASE("BFCombinationFinder") {



    SECTION("Should match levels when other factors are the same") {
        MockGameStats gameStats;
        gameStats.scorer = [] (auto) { return 0; };

        BFCombinationFinder finder(2, gameStats);
        finder.find({1, 2}, {
            PlayerInfo(1, Member::Male, 1, false),
        });
    }
}