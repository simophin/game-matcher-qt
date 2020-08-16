#include <catch2/catch.hpp>

#include "EligiblePlayerFinder.h"

auto createPlayers(unsigned num) {
    QVector<BasePlayerInfo> players;
    players.reserve(num);
    for (unsigned i = 0; i < num; i++) {
        players.push_back(BasePlayerInfo(
                i + 1, (i % 2 == 0) ? BaseMember::Male : BaseMember::Female, 1 + (i % 4)));
    }
    return players;
}

TEST_CASE("EligiblePlayerFinder") {
    auto[members, pastAllocations, playerPerCourt, numCourt] = GENERATE(
            table<QVector<BasePlayerInfo>, QVector<GameAllocation>, unsigned, unsigned>(
                    {
                            {
                                    createPlayers(30),
                                    {
                                            GameAllocation(0, 1, 1, 100),
                                    }
                            },
                    }));
}