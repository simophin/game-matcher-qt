//
// Created by Fanchao Liu on 16/07/20.
//

#include "BFCombinationFinder.h"
#include "MatchingScore.h"

#include <list>
#include <optional>

using nonstd::span;

typedef std::list<PlayerInfo> PlayerInfoList;
typedef typename PlayerInfoList::iterator PlayerInfoIterator;

struct BestCombination {
    std::vector<PlayerInfoIterator> players;
    int score = 0;
    size_t numOptional = 0, numMandatory = 0;
};

struct BestCourtFinder {
    size_t const numPlayerRequired;
    GameStats const &stats;
    unsigned int const minLevel, maxLevel;
    size_t const maxOptionalAllowed, minMandatoryRequired;

    int numEstimated = 0;

    std::vector<PlayerInfoIterator> players;
    size_t numOptional = 0;
    size_t numMandatory = 0;

    std::optional<BestCombination> best;

    void find(PlayerInfoIterator begin, PlayerInfoIterator end) {
        if (players.size() == numPlayerRequired) {
            if (numOptional > maxOptionalAllowed || numMandatory < minMandatoryRequired) {
                return;
            }

            numEstimated++;

            int score = MatchingScore::computeCourtScore(stats, players, minLevel, maxLevel);
            bool update;
            if (!best) {
                best.emplace();
                update = true;
            } else {
                update = score > best->score;
            }

            if (update) {
                best->players.clear();
                best->players.insert(best->players.end(), players.begin(), players.end());
                best->score = score;
                best->numOptional = numOptional;
                best->numMandatory = numMandatory;
            }
        } else {
            while (begin != end) {
                bool isMandatory = begin->mandatory;
                players.emplace_back(begin++);
                if (isMandatory) numMandatory++;
                else numOptional++;

                find(begin, end);

                players.pop_back();
                if (isMandatory) numMandatory--;
                else numOptional--;
            }
        }
    }
};


std::vector<CourtCombinationFinder::CourtAllocation>
BFCombinationFinder::doFind(span<const PlayerInfo> span, size_t numCourtAvailable) const {
    std::list<PlayerInfo> players;
    size_t numOptionalAllowed = 0, numMandatoryRequired = 0;
    for (const auto &item : span) {
        players.emplace_back(item);
        if (!item.mandatory) numOptionalAllowed++;
        else numMandatoryRequired++;
    }

    std::vector<CourtAllocation> result;

    for (int i = 0; i < numCourtAvailable && !players.empty(); i++) {
        BestCourtFinder finder = {(size_t) numPlayersPerCourt_, stats_, minLevel_, maxLevel_, numOptionalAllowed,
                                  numMandatoryRequired};
        finder.find(players.begin(), players.end());
        if (!finder.best) {
            qWarning() << "Unable to find best court";
            break;
        }

        auto &allocation = result.emplace_back();
        for (const auto &player : finder.best->players) {
            allocation.players.emplace_back(*player);
            players.erase(player);
        }
        allocation.quality = finder.best->score;
        numOptionalAllowed -= std::min(numOptionalAllowed, finder.best->numOptional);
        numMandatoryRequired -= std::min(numMandatoryRequired, finder.best->numMandatory);
    }

    return result;
}
