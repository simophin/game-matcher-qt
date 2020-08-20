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
    MockGameStats stats;

    SECTION("First game everyone is equally eligible") {
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

//    auto numPlayers = GENERATE(5, 10, 20, 40);
//    auto numCourt = GENERATE(1, 4, 6, 10);
//    auto playerPerCourt = GENERATE(2, 4);
//
//    auto players = createPlayers(numPlayers);

    SECTION("Non-first game previously off people will be on") {
        stats.numGamesOffByMember = {
                {1, 1},
                {3, 1},
                {5, 2},
                {7, 1},
        };
        for (int i = 0; i < 8; i++) {
            stats.numGamesByMember[i] = 2;
        }

        stats.totalGame = 2;

        // all 'off' people can be on
        auto input = createPlayers(8);
        auto actual = EligiblePlayerFinder::findEligiblePlayers(
                input,
                4, 1, &stats);
        REQUIRE((actual
                 | views::transform([](PlayerInfo p) { return p.memberId; })
                 | to<QVector<MemberId>>()
                 | actions::sort(&compare)
                 )
                == (stats.numGamesOffByMember
                    | views::transform([](auto entry) { return entry.first; })
                    | to<QVector<MemberId>>()
                    | actions::sort(&compare)
                    )
        );

        stats.numGamesOffByMember[2] = 1;
        actual = EligiblePlayerFinder::findEligiblePlayers(
                createPlayers(8),
                4, 1, &stats);
        REQUIRE((actual
                 | views::transform([](PlayerInfo p) { return p.memberId; })
                 | to<QVector<MemberId>>()
                 | actions::sort(&compare)
                )
                == (stats.numGamesOffByMember
                    | views::transform([](auto entry) { return entry.first; })
                    | to<QVector<MemberId>>()
                    | actions::sort(&compare)
                )
        );

        REQUIRE((actual
            | views::filter([](PlayerInfo p) { return p.mandatory; })
            | views::transform([](PlayerInfo p) { return p.memberId; })
            | to<QVector<MemberId>>()) == QVector<MemberId>{ 5 }
            );

        REQUIRE((actual
                 | views::filter([](PlayerInfo p) { return !p.mandatory; })
                 | views::transform([](PlayerInfo p) { return p.memberId; })
                 | to<QVector<MemberId>>()) == QVector<MemberId>{ 1, 2, 3, 7 }
        );
    }

}