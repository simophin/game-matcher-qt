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
#include "CombinationsFinder.h"


static std::vector<PlayerInfo> getEligiblePlayers(nonstd::span<const Member> members,
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

        while (players.size() > cutOffSize) {
            players.pop_back();
        }

        if (players.begin()->eligibilityScore() != lowestScore) {
            // The lowest score players will be re-written to invalid to indicate they are all optional, if there
            // are higher score players.
            for (auto &p : players) {
                if (p.eligibilityScore() == lowestScore) {
                    p.clearEligibilityScore();
                }
            }
        }
    }

    return players;
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

    inline void copy(std::vector<PlayerInfo> &out) const {
        out.clear();
        out.reserve(players.size());
        out.insert(out.end(), players.begin(), players.end());
    }

    inline CourtInfo(nonstd::span<PlayerInfo> players)
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

class BestCourtCombinationsFinder : public CombinationsFinder<CourtInfo> {
    const GameStats &stats_;
    const int uneligibleQuota;
public:
    BestCourtCombinationsFinder(const GameStats &stats, const int numCourtRequired, const int uneligibleQuota)
            : CombinationsFinder(numCourtRequired), stats_(stats), uneligibleQuota(uneligibleQuota) {}

protected:
    std::optional<int> computeScore(nonstd::span<CourtInfo> courts) override {
        // Check for uneligible people
        if (sumBy(courts, numUneligible) > uneligibleQuota) {
            return std::nullopt;
        }

        return reduceCollection(courts, 0, [this](int sum, const CourtInfo &c) {
            return computeCourtScore(stats_, c.players);
        });
    }
};

static std::vector<std::vector<PlayerInfo>> findBestGame(const GameStats &stats,
        nonstd::span<PlayerInfo> players, const int uneligibleQuota,
        const int numPerCourt, const int numCourtRequired) {
    assert(players.size() % numPerCourt == 0);
    BestCourtCombinationsFinder finder(stats, numCourtRequired, uneligibleQuota);
    std::vector<std::vector<PlayerInfo>> bestResult;
    std::optional<int> bestResultScore;

    std::vector<CourtInfo> courts;
    for (size_t i = 0, size = players.size(); i < size; i += numPerCourt) {
        courts.emplace_back(players.subspan(i, numPerCourt));
    }
    bestResult.resize(courts.size());

    for (size_t i = 0, numCourts = players.size() / numPerCourt; i < numCourts; i++) {
        for (size_t j = 0; j < numPerCourt; j++) {
            auto firstIndex = i * numPerCourt + j;

            for (size_t k = i + 1; k < numCourts; k++) {
                for (size_t m = 0; m < numPerCourt; m++) {
                    auto secondIndex = k * numPerCourt + m;
                    swap(players[firstIndex], players[secondIndex]);
                    if (auto result = finder.find(courts.begin(), courts.end())) {
                        if (!bestResultScore || result->score > *bestResultScore) {
                            bestResultScore = result->score;
                            for (size_t n = 0, size = result->data.size(); n < size; n++) {
                                result->data[n].copy(bestResult[0]);
                            }
                        }
                    }
                    swap(players[firstIndex], players[secondIndex]);
                }
            }
        }
    }

    if (bestResultScore) return bestResult;
    return {};
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

