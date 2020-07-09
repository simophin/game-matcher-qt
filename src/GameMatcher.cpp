#include "GameMatcher.h"

#include <list>
#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>
#include <random>

#include "GameMatcherInternal.h"
#include "ClubRepository.h"
#include "CollectionUtils.h"


static PlayerList getEligiblePlayers(nonstd::span<const Member> members,
                                     int numMaxSeats,
                                     const GameStats &stats, int randomSeed) {
    std::vector<PlayerInfo> players;
    if (members.empty()) return {};

    for (const auto &member : members) {
        players.emplace_back(member, stats.numGamesOff(member.id));
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(players.begin(), players.end(), std::default_random_engine(randomSeed));

    // Sort by eligibility score max to min
    std::sort(players.begin(), players.end(), [](auto &a, auto &b) {
        return b.eligibilityScore() < a.eligibilityScore();
    });

    if (players.size() > numMaxSeats && numMaxSeats > 0) {
        // Find lowest score and we will cut off from there
        int cutOffSize = numMaxSeats;
        const auto lowestScore = players[numMaxSeats - 1].eligibilityScore();
        for (; cutOffSize < players.size(); cutOffSize++) {
            if (players[cutOffSize].eligibilityScore() != lowestScore) break;
        }

        PlayerList playerList(players.begin(), players.begin() + cutOffSize);

        if (playerList.begin()->eligibilityScore() != lowestScore) {
            // The lowest score players will be re-written to invalid to indicate they are all optional, if there
            // are higher score players.
            for (auto &p : playerList) {
                if (p.eligibilityScore() == lowestScore) {
                    p.clearEligibilityScore();
                }
            }
        }
        return playerList;
    }

    return PlayerList(players.begin(), players.end());
}


template<typename Col>
static int levelRangeScore(const Col &members) {
    if (members.size() < 2) return 0;

    int min = levelMax + 1, max = levelMin - 1;
    for (const PlayerInfo &m : members) {
        auto level = m.level();
        if (level > max) max = level;
        if (level < min) min = level;
    }

    return (max - min) * 100 / (levelMax - levelMin);
}

template<typename Col>
static int levelStdVarianceScore(const Col &members) {
    if (members.size() < 2) return 0;

    const int sum = reduceCollection(members, 0, [](auto sum, const PlayerInfo &info) {
        return sum + info.level();
    });
    const double average = static_cast<double>(sum) / members.size();

    return std::sqrt(reduceCollection(members, 0.0, [average](auto sum, const PlayerInfo &info) {
        auto diff = info.level() - average;
        return sum + diff * diff;
    }) / (members.size() - 1)) * 100 / (levelMax - levelMin);
}

template<typename Col>
static int genderSimilarityScore(const Col &players) {
    if (players.size() < 2) return 100;

    int numMales = std::count_if(players.begin(), players.end(), [](const PlayerInfo &p) {
        return p.gender() == genderMale;
    });

    int numFemales = players.size() - numMales;
    if (numMales == 0 || numMales == players.size() || numFemales == numMales) return 100;

    int maxDiff = players.size() - 2;
    return (maxDiff - std::abs(numMales - numFemales)) * 100 / maxDiff;
}

struct CourtInfo {
    nonstd::span<PlayerInfo> const players;
    int const numUneligible;
    int score = 0;

    inline CourtInfo(const nonstd::span<PlayerInfo> &players)
            : players(players), numUneligible(reduceCollection(players, 0,
                                                               [](int sum, const PlayerInfo &p) {
                                                                   return sum +
                                                                          (p.eligibilityScore().has_value() ? 1 : 0);
                                                               })) {}

};

template<typename Col>
static int computeCourtScore(const GameStats &stats, const Col &players) {
    int score = 0;
    score -= stats.similarityScore(players) * 4;
    score -= levelStdVarianceScore(players) * 2;
    score -= levelRangeScore(players) * 2;
    score += genderSimilarityScore(players);
    return score;
}

struct BestCourtCombinationsResult {
    QVector<CourtInfo> courts;
    int score;
};

class BestCourtCombinationsFinder {
    const int numCourtRequired;
    int uneligiblePlayerQuota;

    QVector<CourtInfo> out;

    QVector<CourtInfo> best;
    int bestScore = 0;

    void doFind(nonstd::span<CourtInfo> courts) {
        if (out.size() == numCourtRequired) {

        }
    }

public:
    BestCourtCombinationsFinder(const int numCourtRequired, int uneligibleQuota)
            : numCourtRequired(numCourtRequired),
              uneligiblePlayerQuota(uneligibleQuota) {}


    std::optional<BestCourtCombinationsResult> find(nonstd::span<CourtInfo> courts) {

    }
};

template<typename Col>
static std::optional<BestCourtCombinationsResult> findBestCourtCombinations(const GameStats &stats, const int uneligibleQuota,
                                                    const int numCourtRequired, const Col &courts) {
    QVector<CourtInfo> result;
    result.reserve(numCourtRequired);

    int score = 0;
    for (const CourtInfo &court : courts) {
        if (court.numUneligible <= uneligibleQuota) {
            result.append(court);
        }
    }

    if (result.size() < numCourtRequired) {
        // It's impossible to arrange a game with these courts
        return std::nullopt;
    }

    int totalUneligible = 0;

    for (auto &item : result) {
        item.score = computeCourtScore(stats, item.players);
        totalUneligible += item.numUneligible;
    }

    std::sort(result.begin(), result.end(), [](const auto &a, const auto &b) {
        return b.score < a.score;
    });

    if (totalUneligible <= uneligibleQuota) {
        while (result.size() >= numCourtRequired) result.removeLast();
        return result;
    }

}


QVector<GameAllocation> GameMatcher::match(
        const QVector<GameAllocation> &pastAllocation,
        const QVector<Member> &members,
        QVector<CourtId> courts,
        size_t playerPerCourt,
        int seed) {
    qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size()
             << " players and "
             << courts.size() << " courts";

    QVector<GameAllocation> result;

    const size_t numMaxSeats = std::min(playerPerCourt * courts.size(),
                                        members.size() / playerPerCourt * courts.size());
    if (pastAllocation.isEmpty()) {
        auto sortedMembers = members;
        std::shuffle(sortedMembers.begin(), sortedMembers.end(), std::default_random_engine(seed));

        // First game, sort players by level and only take the top to down people
        std::sort(sortedMembers.begin(), sortedMembers.end(), [](const auto &a, const auto &b) {
            return b.level < a.level;
        });

        const auto numMembers = std::min(numMaxSeats, sortedMembers.size() / playerPerCourt * playerPerCourt);
        result.reserve(numMembers);
        auto courtIter = courts.begin();
        for (int i = 0; i < numMembers; i++) {
            result.append(GameAllocation(0, *courtIter, sortedMembers[i].id));

            // Advance to next court when this one is full
            if ((i + 1) % playerPerCourt == 0) {
                courtIter++;
            }
        }
        return result;
    }

    GameStats stats(pastAllocation);


    // Find eligible players
    auto eligiblePlayers = getEligiblePlayers(members, numMaxSeats, stats, seed);

    // How many unqualified people can we use?
    const auto numMustOn = std::count_if(eligiblePlayers.cbegin(), eligiblePlayers.cend(), [](auto &p) {
        return p.eligibilityScore().has_value();
    });

    size_t unqualifiedQuota = numMaxSeats > numMustOn ? (numMaxSeats - numMustOn) : 0;

//    std::shuffle(courts.begin(), courts.end(), std::default_random_engine(seed + 1));

//    for (const auto &courtId : courts) {
//        if (eligiblePlayers.size() < playerPerCourt) break;
//        if (eligiblePlayers.size() == playerPerCourt) {
//            for (const auto &player : eligiblePlayers) {
//                result.append(GameAllocation(0, courtId, player.id()));
//            }
//            break;
//        }
//
//        ComputeContext ctx = {stats, playerPerCourt, unqualifiedQuota, eligiblePlayers.cend()};
//        ctx.out.reserve(playerPerCourt);
//
//        computeCourtScore(ctx, eligiblePlayers.begin());
//        if (!ctx.bestScore) {
//            qWarning() << "Unable to find best score";
//            return result;
//        }
//
//        unqualifiedQuota = ctx.bestScore->unqualifiedQuotaLeft;
//
//        for (const auto &p : ctx.bestScore->combination) {
//            result.append(GameAllocation(0, courtId, p.id()));
//            eligiblePlayers.erase(p);
//        }
//    }
    qDebug() << "Matched result has " << result.size() << " allocations";

    return result;

}

