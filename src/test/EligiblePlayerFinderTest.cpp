#include <catch2/catch.hpp>

#include "EligiblePlayerFinder.h"
#include "MockGameStats.h"

#include <range/v3/view.hpp>
#include <range/v3/action.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/access.hpp>

using namespace ranges;

auto createPlayers(unsigned num) {
    QVector<BasePlayerInfo> players;
    players.reserve(num);
    for (unsigned i = 0; i < num; i++) {
        players.push_back(BasePlayerInfo(
                i + 1, (i % 2 == 0) ? BaseMember::Male : BaseMember::Female, 1 + (i % 4)));
    }
    return players;
}

static bool compare(const MemberId &lhs, const MemberId &rhs) {
    return lhs < rhs;
}

TEST_CASE("EligiblePlayerFinder") {
    SECTION("First game everyone is equally eligible") {
        MockGameStats stats;
        auto numPlayers = GENERATE(0, 1, 4, 10, 20);
        auto input = createPlayers(numPlayers);
        QVector<PlayerInfo> output;
        for (const auto &p :  input) {
            output.push_back(PlayerInfo(p, false));
        }

        auto[playerPerCourt, numCourt, inputStats, expected] = GENERATE_REF(
                table<unsigned, unsigned, const GameStats *, QVector<PlayerInfo>>(
                        {
                                {0, 4, &stats,  {},},
                                {4, 0, &stats,  {},},
                                {1, 1, nullptr, output,},
                                {1, 1, &stats,  output,},
                                {4, 4, &stats,  output,},
                        }));

        auto actual = EligiblePlayerFinder::findEligiblePlayers(input, playerPerCourt, numCourt, inputStats);
        REQUIRE(actual == expected);
    }

    SECTION("Non-first game previously off people will be on") {
        auto[numCourt, players, numGamesOff, numGames, mandatoryIds, optionalIds] = GENERATE(
                table<int, QVector<BasePlayerInfo>, std::map<MemberId, int>, std::map<MemberId, int>, QVector<MemberId>, QVector<MemberId>>(
                        {
                                {
                                        1,
                                        createPlayers(8),
                                        {{1, 1}, {3, 1}, {5, 1}, {7, 1}},
                                        {},
                                        {},
                                        {1, 3, 5, 7},
                                },
                                {       1,
                                        createPlayers(8),
                                        {{1, 1}, {3, 1}, {2, 2}, {5, 1}, {7, 1}},
                                        {},
                                        {2},
                                        {1, 3, 5, 7},
                                },
                                {       2,
                                        createPlayers(8),
                                        {{1, 1}, {3, 1}, {2, 2}, {5, 1}, {7, 1}},
                                        {},
                                        {},
                                        {1, 2, 3, 4, 5, 6, 7, 8},
                                },
                        }
                ));

        MockGameStats stats;
        stats.numGamesOffByMember = numGamesOff;
        stats.totalGame = 2;
        stats.numGamesByMember = numGames;

        auto actual = EligiblePlayerFinder::findEligiblePlayers(players, 4, numCourt, &stats);
        REQUIRE(
                (actual
                 | views::filter([](PlayerInfo p) { return p.mandatory; })
                 | views::transform([](PlayerInfo p) { return p.memberId; })
                 | to<QVector<MemberId>>()
                 | actions::sort(&compare))
                == mandatoryIds
        );

        REQUIRE(
                (actual
                 | views::filter([](PlayerInfo p) { return !p.mandatory; })
                 | views::transform([](PlayerInfo p) { return p.memberId; })
                 | to<QVector<MemberId>>()
                 | actions::sort(&compare))
                == optionalIds
        );
    }

}