//
// Created by Fanchao Liu on 30/07/20.
//

#include "CourtFinderV1.h"
#include "MatchingScore.h"
#include "FastVector.h"

#include <functional>
#include <QSet>

#include "LinkedEntry.h"

using nonstd::span;
using std::vector;

struct PlayerInfoData : public LinkedEntry<PlayerInfoData>, PlayerInfo {
    using PlayerInfo::PlayerInfo;

    PlayerInfoData(const PlayerInfo &rhs)
        :PlayerInfoData(rhs.memberId, rhs.gender, rhs.level, rhs.mandatory) {}
};

typedef std::function<void(span<PlayerInfo *>)> CourtArrangementCallback;


inline QDebug operator<<(QDebug debug, const vector<const PlayerInfo *> &players) {
    QDebugStateSaver saver(debug);
    auto &d = debug.nospace().noquote();
    d << "[";
    for (const auto &player : players) {
        d << *player << ", ";
    }
    d << "]";
    return debug;
}

struct CourtArrangementFindContext {
    CourtArrangementCallback const fn;
    const size_t numMandatoryRequired, numOptionalAllowed;
    const size_t playerPerCourt, numCourt;

    const size_t numPlayersOnCourts;

    PlayerInfoData *availablePlayerHead;

    CourtArrangementFindContext(CourtArrangementCallback &&fn, const size_t numMandatoryRequired,
                                const size_t numOptionalAllowed, const size_t playerPerCourt, const size_t numCourt,
                                PlayerInfoData *playerHead, size_t numPlayers)
            : fn(std::move(fn)), numMandatoryRequired(numMandatoryRequired), numOptionalAllowed(numOptionalAllowed),
              playerPerCourt(playerPerCourt), numCourt(numCourt), availablePlayerHead(playerHead),
              numPlayersOnCourts(
                      std::min(numPlayers, playerPerCourt * numCourt) / playerPerCourt * playerPerCourt) {
        qDebug() << "Finding matches on " << numPlayers << " players";
    }

    size_t numMandatory = 0, numOptional = 0;
    FastVector<PlayerInfo *> arrangement;

    size_t numInspect = 0;

    void find(PlayerInfoData *curr) {
        if (arrangement.size() == numPlayersOnCourts) {
            numInspect++;
            if (numMandatory >= numMandatoryRequired) {
//                qDebug() << "Inspecting arrangement " << arrangement;
//                fn(span<PlayerInfo*>(&arrangement[0], &arrangement[arrangement.size() - 1]));
            }
            return;
        }

        if (!arrangement.empty() && (arrangement.size() % playerPerCourt) == 0) {
            curr = availablePlayerHead;
        }

        while (curr) {
            bool wasHead = curr->isHead();
            auto prev = curr->prev;
            auto next = curr->next;
            curr->remove();
            if (wasHead) availablePlayerHead = next;

            if (curr->mandatory) numMandatory++;
            else numOptional++;

            if (numOptional < numOptionalAllowed) {
                arrangement.push_back(curr);
//                qDebug() << "Adding player " << *curr << " to arrangement";
                find(next);
                arrangement.pop_back();
            }

            if (curr->mandatory) numMandatory--;
            else numOptional--;

            assert(prev || next);

            if (prev) prev->insertAfter(curr);
            else if (next) next->insertBefore(curr);

            if (wasHead) {
                availablePlayerHead = curr;
            }

            curr = next;
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
    std::vector<PlayerInfoData> list(players.begin(), players.end());
    for (size_t i = 0, size = list.size(); i < size; i++) {
        if (i > 0) list[i].prev = &list[i - 1];
        if (i < size - 1) list[i].next = &list[i + 1];
    }

    CourtArrangementFindContext ctx(std::move(cb), numMandatoryRequired, numOptionalAllowed, playerPerCourt, numCourt,
                                    &list[0], list.size());
    ctx.find(&list[0]);
    qDebug() << "Generated " << ctx.numInspect << " arrangements";
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
            [&](span<PlayerInfo *> arrangement) {
//                thread_local vector<int> tmpCourtScores;
//                int totalScore = computeArrangementScore(stats_, numPlayersPerCourt_, arrangement, minLevel_, maxLevel_,
//                                                         tmpCourtScores);
//
//                if (!result.totalScore || (*result.totalScore) < totalScore) {
//                    result.arrangement.clear();
//                    result.arrangement.insert(result.arrangement.end(), arrangement.begin(), arrangement.end());
//                    result.courtScores = tmpCourtScores;
//                    result.totalScore = totalScore;
//                }
            });

    return vector<CourtAllocation>();
}
