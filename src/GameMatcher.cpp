#include "GameMatcher.h"

#include <list>
#include <vector>

#include <QtDebug>
#include <QHash>

#include <algorithm>
#include <random>

#include "GameMatcherInternal.h"
#include "ClubRepository.h"


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
        return b.eligibilityScore < a.eligibilityScore;
    });

    if (players.size() > numMaxSeats && numMaxSeats > 0) {
        // Find lowest score and we will cut off from there
        int cutOffSize = numMaxSeats;
        const auto lowestScore = players[numMaxSeats - 1].eligibilityScore;
        for (; cutOffSize < players.size(); cutOffSize++) {
            if (players[cutOffSize].eligibilityScore != lowestScore) break;
        }

        PlayerList playerList(players.begin(), players.begin() + cutOffSize);

        if (playerList.begin()->eligibilityScore != lowestScore) {
            // The lowest score players will be re-written to invalid to indicate they are all optional, if there
            // are higher score players.
            for (auto &p : playerList) {
                if (p.eligibilityScore == lowestScore) {
                    p.eligibilityScore = std::nullopt;
                }
            }
        }
        return playerList;
    }

    return PlayerList(players.begin(), players.end());
}

struct BestScore {
    QVector<PlayerListIterator> combination;
    int score;
    size_t unqualifiedQuotaLeft;
};

struct ComputeContext {
    const GameStats &stats;
    const size_t numPlayersRequired;
    size_t unqualifiedQuota;
    const PlayerListIterator endOfPlayerList;

    QVector<PlayerListIterator> out;
    std::optional<BestScore> bestScore;
};

static int levelRangeScore(nonstd::span<PlayerListIterator> members) {
    if (members.size() < 2) return 0;
    
    int min = levelMax + 1, max = levelMin - 1;
    for (const auto &m : members) {
        auto level = m->member.level;
        if (level > max) max = level;
        if (level < min) min = level;
    }

    return (max - min) * 100 / (levelMax - levelMin);
}

static int levelStdVarianceScore(nonstd::span<PlayerListIterator> members) {
    if (members.size() < 2) return 0;

    const int sum = std::reduce(members.begin(), members.end(), 0, [](auto sum, auto info) {
        return sum + info->member.level;
    });
    const double average = static_cast<double>(sum) / members.size();

    return std::sqrt(std::reduce(members.begin(), members.end(), 0.0, [average](auto sum, auto info) {
        auto diff = info->member.level - average;
        return sum + diff * diff;
    }) / (members.size() - 1)) * 100 / (levelMax - levelMin);
}

static int genderSimilarityScore(nonstd::span<PlayerListIterator> players) {
    if (players.size() < 2) return 100;

    int numMales = std::count_if(players.begin(), players.end(), [](PlayerListIterator p) {
        return p->member.gender == genderMale;
    });

    int numFemales = players.size() - numMales;
    if (numMales == 0 || numMales == players.size() || numFemales == numMales) return 100;

    int maxDiff = players.size() - 2;
    return (maxDiff - std::abs(numMales - numFemales)) * 100 / maxDiff;
}

static void computeCourtScore(ComputeContext &ctx,
                              PlayerListIterator players) {
    if (ctx.out.size() == ctx.numPlayersRequired) {
        auto score = 0;

        score -= ctx.stats.similarityScore(ctx.out) * 4;
        score -= levelStdVarianceScore(ctx.out) * 2;
        score -= levelRangeScore(ctx.out) * 2;
        score += genderSimilarityScore(ctx.out);

        if (!ctx.bestScore || ctx.bestScore->score < score) {
            ctx.bestScore = {ctx.out, score, ctx.unqualifiedQuota};
        }
    } else {
        while (players != ctx.endOfPlayerList) {
            bool mustOn = players->eligibilityScore.has_value();
            if (!mustOn) {
                if (ctx.unqualifiedQuota == 0) {
                    players++;
                    continue;
                }

                ctx.unqualifiedQuota--;
            }
            ctx.out.append(players);
            
            computeCourtScore(ctx, ++players);
            ctx.out.removeLast();
            if (!mustOn) ctx.unqualifiedQuota++;
        }
    }
}


QVector<GameAllocation> GameMatcher::match(
        const QVector<GameAllocation> &pastAllocation,
        const QVector<Member> & members,
        QVector<CourtId> courts,
        size_t playerPerCourt,
        int seed) {
    qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size()
             << " players and "
             << courts.size() << " courts";

    QVector<GameAllocation> result;

    const size_t numMaxSeats = playerPerCourt * courts.size();
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
        return p.eligibilityScore.has_value();
    });

    size_t unqualifiedQuota = numMaxSeats > numMustOn ? (numMaxSeats - numMustOn) : 0;

//    std::shuffle(courts.begin(), courts.end(), std::default_random_engine(seed + 1));

    for (const auto &courtId : courts) {
        ComputeContext ctx = {stats, playerPerCourt, unqualifiedQuota, eligiblePlayers.cend()};
        ctx.out.reserve(playerPerCourt);

        computeCourtScore(ctx, eligiblePlayers.begin());
        if (!ctx.bestScore) {
            qWarning() << "Unable to find best score";
            return result;
        }

        unqualifiedQuota = ctx.bestScore->unqualifiedQuotaLeft;

        for (const auto &p : ctx.bestScore->combination) {
            result.append(GameAllocation(0, courtId, p->member.id));
            eligiblePlayers.erase(p);
        }
    }
    qDebug() << "Matched result has " << result.size() << " allocations";

    return result;

}

