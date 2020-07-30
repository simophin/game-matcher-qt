//
// Created by Fanchao Liu on 30/07/20.
//

#include "CourtFinderV1.h"
#include "MatchingScore.h"

#include <functional>
#include <QSet>

using nonstd::span;
using std::vector;

typedef std::function<void(span<const PlayerInfo *>)> CourtArrangementCallback;

struct CourtArrangementFindContext {
    CourtArrangementCallback fn;
    size_t numMandatoryRequired = 0, numOptionalAllowed = 0;
    size_t playerPerCourt = 0, numCourt = 0;

    size_t numMandatory = 0, numOptional = 0;
    vector<const PlayerInfo *> arrangement;
    QSet<MemberId> availableMemberIds;

    void find(span<const PlayerInfo> players) {
        if (arrangement.size() > 0 &&
                (arrangement.size() == playerPerCourt * numCourt) ||
                (arrangement.size() % playerPerCourt == 0 && players.size() < playerPerCourt)) {
            if (numMandatory >= numMandatoryRequired) {
                fn(arrangement);
            }
        } else {
            for (size_t i = 0, size = players.size(); i < size; i++) {
                auto &p = players[i];
                if ((!p.mandatory && (numOptional + 1) <= numOptionalAllowed) || p.mandatory) {
                    if (p.mandatory) numMandatory++;
                    else numOptional++;

                    arrangement.push_back(&p);
                    find(players.subspan(i+1));
                    arrangement.pop_back();

                    if (p.mandatory) numMandatory--;
                    else numOptional--;
                }
            }
        }
    }
};


static void forEachCourtArrangement(
        span<const PlayerInfo> players,
        size_t playerPerCourt, size_t numCourt,
        CourtArrangementCallback cb) {
    CourtArrangementFindContext ctx;
    ctx.fn = std::move(cb);
    ctx.playerPerCourt = playerPerCourt;
    ctx.numCourt = numCourt;
    for (const auto &player : players) {
        if (player.mandatory) ctx.numMandatoryRequired++;
        else ctx.numOptionalAllowed++;

        ctx.availableMemberIds.insert(player.memberId);
    }
}

static int computeArrangementScore(
        const GameStats &stats, size_t numPerCourt, span<const PlayerInfo *> players,
        int minLevel, int maxLevel,
        vector<int> &courtScores) {
    courtScores.clear();

    int totalScore = 0;
    const size_t numCourt = players.size() / numPerCourt;
    courtScores.reserve(numCourt);
    for (size_t i = 0; i < numCourt; i++) {
        auto score = MatchingScore::computeCourtScore(stats, players.subspan(i * numPerCourt, numPerCourt), minLevel,
                                                      maxLevel);
        courtScores.push_back(score);
        totalScore += score;
    }
    return totalScore;
}

vector<CourtFinderV1::CourtAllocation>
CourtFinderV1::doFind(span<const PlayerInfo> players, size_t numCourtAvailable) const {
    struct AllocationResult {
        vector<const PlayerInfo *> arrangement;
        vector<int> courtScores;
        std::optional<int> totalScore;
    };

    AllocationResult result;

    forEachCourtArrangement(
            players,
            numPlayersPerCourt_,
            numCourtAvailable,
            [&](span<const PlayerInfo *> arrangement) {
                thread_local vector<int> tmpCourtScores;
                int totalScore = computeArrangementScore(stats_, numPlayersPerCourt_, arrangement, minLevel_, maxLevel_,
                                                         tmpCourtScores);

                if (!result.totalScore || (*result.totalScore) < totalScore) {
                    result.arrangement.clear();
                    result.arrangement.insert(result.arrangement.end(), arrangement.begin(), arrangement.end());
                    result.courtScores = tmpCourtScores;
                    result.totalScore = totalScore;
                }
            });

    return vector<CourtAllocation>();
}
