#include "GameMatcher.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <list>
#include <vector>
#include <QRandomGenerator>

#include <QtDebug>
#include <QHash>

#include <algorithm>

#include "GameStats.h"
#include "ClubRepository.h"

#include "span.h"


static std::list<PlayerInfo> getEligiblePlayers(const QVector<MemberInfo> &members,
                               int numSeats,
                               const GameStats &stats, int randomSeed) {
    std::vector<PlayerInfo> players;
    if (members.empty()) return {};
    
    for (const auto &member : members) {
        players.emplace_back(member, member.numGames < 0 ? 0 : (-member.numGames));
    }

    // Shuffle the list so we don't end up using the same order withing same level.
    std::shuffle(players.begin(), players.end(), std::default_random_engine(randomSeed));

    // Sort by eligibility score max to min
    std::sort(players.begin(), players.end(), [](auto &a, auto &b) {
        return b.eligibilityScore < a.eligibilityScore;
    });

    if (players.size() > numSeats && numSeats > 0) {
        // Find lowest score and we will cut off from there
        int cutOffSize = numSeats;
        const auto lowestScore = players[numSeats - 1].eligibilityScore;
        for (; cutOffSize < players.size(); cutOffSize++) {
            if (players[cutOffSize].eligibilityScore != lowestScore) break;
        }

        std::list<PlayerInfo> playerList(players.begin(), players.begin() + cutOffSize);

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

    return std::list<PlayerInfo>(players.begin(), players.end());
}

struct BestScore {
    QVector<std::list<PlayerInfo>::const_iterator> combination;
    int score;
    size_t unqualifiedQuotaUsed;
};

struct ComputeContext {
    const GameStats &stats;
    const size_t numPlayersRequired;
    const size_t unqualifiedQuota;
    const std::list<PlayerInfo>::const_iterator endOfPlayerList;

    QVector<std::list<PlayerInfo>::const_iterator> out;
    std::optional<BestScore> bestScore;
};

static int levelVariance(const QVector<std::list<PlayerInfo>::const_iterator> &members) {
    if (members.empty()) return 0;

    int sum = std::reduce(members.begin(), members.end(), 0, [](auto sum, auto info) {
        return sum + info->member.level;
    });
    auto average = sum / members.size();
    return std::reduce(members.begin(), members.end(), 0, [average](auto sum, auto info) {
        auto diff = info->member.level - average;
        return sum + diff * diff;
    });
}

static void computeCourtScore(ComputeContext &ctx,
                              std::list<PlayerInfo>::const_iterator players) {
    if (ctx.out.size() == ctx.numPlayersRequired) {
        size_t numUnqualified = std::count_if(ctx.out.begin(), ctx.out.end(), [](auto &p) {
            return !p->eligibilityScore.has_value();
        });

        if (numUnqualified > ctx.unqualifiedQuota) {
            return;
        }

        auto score = -ctx.stats.similarityWithPast(ctx.out) - levelVariance(ctx.out);
        if (!ctx.bestScore || ctx.bestScore->score < score) {
            ctx.bestScore = {ctx.out, score, numUnqualified};
        }
    } else {
        while (players != ctx.endOfPlayerList) {
            ctx.out.append(players);
            computeCourtScore(ctx, ++players);
            ctx.out.removeLast();
        }
    }
}


QFuture<QVector<GameAllocation>> GameMatcher::match(
        const QVector<GameAllocation> &pastAllocation,
        const QVector<MemberInfo> &members,
        const QVector<CourtId> &courts,
        size_t playerPerCourt,
        int seed) {
    return QtConcurrent::run([=]() mutable {
        qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size()
                 << " players and "
                 << courts.size() << " courts";

        QVector<GameAllocation> result;

        const auto numMaxSeats = playerPerCourt * courts.size();
        if (pastAllocation.isEmpty()) {
            // First game, sort players by level and only take the top to down people
            std::sort(const_cast<QVector<MemberInfo> &>(members).begin(),
                      const_cast<QVector<MemberInfo> &>(members).end(), [](const auto &a, const auto &b) {
                        return b.level < a.level;
                    });

            const auto numMembers = std::min(numMaxSeats, members.size() / playerPerCourt * playerPerCourt);
            result.reserve(numMembers);
            auto courtIter = courts.begin();
            for (int i = 0; i < numMembers; i++) {
                result.append(GameAllocation(0, *courtIter, members[i].id));

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
        size_t unqualifiedQuota =
                numMaxSeats - std::count_if(eligiblePlayers.cbegin(), eligiblePlayers.cend(), [](auto &p) {
                    return p.eligibilityScore;
                });

        for (const auto &courtId : courts) {
            ComputeContext ctx = {stats, playerPerCourt, unqualifiedQuota, eligiblePlayers.cend()};
            ctx.out.reserve(playerPerCourt);

            computeCourtScore(ctx, eligiblePlayers.begin());
            if (!ctx.bestScore) {
                qWarning() << "Unable to find best score";
                return result;
            }

            unqualifiedQuota -= ctx.bestScore->unqualifiedQuotaUsed;

            for (const auto &p : ctx.bestScore->combination) {
                result.append(GameAllocation(0, courtId, p->member.id));
                eligiblePlayers.erase(p);
            }
        }
        qDebug() << "Matched result has " << result.size() << " allocations";

        return result;
    });

}

