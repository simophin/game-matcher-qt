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


struct PlayerInfo {
    MemberInfo member;
    std::optional<int> eligibilityScore;
    int matchingScore = 0;
};


static QVector<PlayerInfo> getEligiblePlayers(const QVector<MemberInfo> &members,
                                                int numSeats,
                                                const GameStats &stats, int randomSeed) {
    QVector<PlayerInfo> players;
    players.resize(members.size());
    for (const auto &member : members) {
        players.push_back({
            member,
            member.numGames < 0 ? 0 : (-member.numGames)
        });
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

        players.resize(cutOffSize);

        if (players.begin()->eligibilityScore != lowestScore) {
            // The lowest score players will be re-written to invalid to indicate they are all optional, if there
            // are higher score players.
            for (auto &p : players) {
                if (p.eligibilityScore == lowestScore) {
                    p.eligibilityScore = std::nullopt;
                }
            }
        }
    }

    return players;
}

PlayerInfo takeRandomEligiblePlayer(QVector<PlayerInfo> &data, QRandomGenerator &rnd) {
    const int start = rnd.bounded(data.size());
    auto &player = data.at(start);
    if (player.eligibilityScore) return data.takeAt(start);

    // Search forward
    for (int i = start; i < data.size(); i++) {
        if (data.at(i).eligibilityScore) {
            return data.takeAt(i);
        }
    }

    // Search backward
    for (int i = start; i >= 0; i--) {
        if (data.at(i).eligibilityScore) {
            return data.takeAt(i);
        }
    }

    // No eligible player found just return the non-eligible one.
    return data.takeAt(start);
}

QFuture<QVector<GameAllocation>> GameMatcher::match(
        const QVector<GameAllocation> &pastAllocation,
        const QVector<MemberInfo> &members,
        const QVector<CourtId> &courts,
        int playerPerCourt,
        int seed) {
    return QtConcurrent::run([=] {
        qDebug() << "Matching using " << pastAllocation.size() << " past allocations, " << members.size() << " players and "
                 << courts.size() << " courts";
        GameStats stats(pastAllocation, members);
        const auto numMaxSeats = playerPerCourt * courts.size();

        // Find eligible players
        auto eligiblePlayers = getEligiblePlayers(members, numMaxSeats, stats, seed);

        // Sort players by level (min -> max)
        std::sort(eligiblePlayers.begin(), eligiblePlayers.end(), [](const auto &a, const auto &b) {
            return a.member.level < b.member.level;
        });

        QRandomGenerator generator(seed);
        QVector<GameAllocation> result;

        // How many unqualified people can we use?
        int unqualifiedQuota = numMaxSeats - std::count_if(eligiblePlayers.cbegin(), eligiblePlayers.cend(), [](auto &p) {
            return p.eligibilityScore;
        });
        
        for (const auto &court : courts) {
            auto player = takeRandomEligiblePlayer(eligiblePlayers, generator);

            // 1. Search nearby to see if some people haven't matched together
        }
        qDebug() << "Matched result has " << result.size() << " allocations";

        return result;
    });

}

