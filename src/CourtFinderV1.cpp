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
    CourtArrangementCallback const fn;
    const size_t numMandatoryRequired, numOptionalAllowed;
    const size_t playerPerCourt, numCourt;

    const size_t numPlayersOnCourts;

    std::list<const PlayerInfo *> &inputList;

    CourtArrangementFindContext(CourtArrangementCallback &&fn, const size_t numMandatoryRequired,
                                const size_t numOptionalAllowed, const size_t playerPerCourt, const size_t numCourt,
                                std::list<const PlayerInfo *> &inputList)
            : fn(std::move(fn)), numMandatoryRequired(numMandatoryRequired), numOptionalAllowed(numOptionalAllowed),
              playerPerCourt(playerPerCourt), numCourt(numCourt), inputList(inputList),
              numPlayersOnCourts(
                      std::min(inputList.size(), playerPerCourt * numCourt) / playerPerCourt * playerPerCourt) {}

    size_t numMandatory = 0, numOptional = 0;
    vector<const PlayerInfo *> arrangement;

    void find(std::list<const PlayerInfo *>::iterator begin) {
        if (arrangement.size() == numPlayersOnCourts) {
            if (numMandatory >= numMandatoryRequired) {
                fn(arrangement);
            }
            return;
        }

        if (arrangement.size() < numPlayersOnCourts && (arrangement.size() % playerPerCourt) == 0) {
            begin = inputList.begin();
        }

        for (auto iter = begin; iter != inputList.end(); ++iter) {
            auto player = *iter;
            iter = inputList.erase(iter);

            if (player->mandatory) numMandatory++;
            else numOptional++;

            if (numOptional < numOptionalAllowed) {
                arrangement.push_back(player);
                find(iter);
            }

            if (player->mandatory) numMandatory--;
            else numOptional--;

            inputList.insert(iter, player);
        }
    }
};


static void forEachCourtArrangement(
        span<const PlayerInfo> players,
        size_t playerPerCourt, size_t numCourt,
        CourtArrangementCallback cb) {
    size_t numMandatoryRequired = 0, numOptionalAllowed = 0;
    for (const auto &player : players) {
        if (player.mandatory) numMandatoryRequired++;
        else numOptionalAllowed++;
    }
    std::list<const PlayerInfo *> list;
    for (const auto &p : players) {
        list.push_back(&p);
    }

    CourtArrangementFindContext ctx(std::move(cb), numMandatoryRequired, numOptionalAllowed, playerPerCourt, numCourt, list);
    ctx.find(list.begin());
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
