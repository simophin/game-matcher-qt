//
// Created by Fanchao Liu on 16/07/20.
//

#include "BruteForceCombinationFinder.h"
#include "MatchingScore.h"

#include <list>
#include <optional>

using nonstd::span;

typedef std::list<PlayerInfo> PlayerInfoList;
typedef typename PlayerInfoList::iterator PlayerInfoIterator;

struct BestCombination {
    std::vector<PlayerInfoIterator> players;
    int score = 0;
    int numOptional = 0;
};

struct BestCourtFinder {
    int const numRequired;
    GameStats const &stats;
    unsigned int const minLevel, maxLevel;
    int const numMaxOptional;

    int numEstimated = 0;

    std::vector<PlayerInfoIterator> players;
    int numOptional = 0;

    std::optional<BestCombination> best;

    void find(PlayerInfoIterator begin, PlayerInfoIterator end) {
        if (players.size() == numRequired) {
            if (numOptional > numMaxOptional) {
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
            }
        } else {
            while (begin != end) {
                bool isOptional = !begin->mandatory;
                players.emplace_back(begin++);
                if (isOptional) numOptional++;
                find(begin, end);
                players.pop_back();
                if (isOptional) numOptional--;
            }
        }
    }
};


std::vector<CourtCombinationFinder::CourtAllocation>
BruteForceCombinationFinder::doFind(span<const PlayerInfo> span, size_t numCourtAvailable) const {
    std::list<PlayerInfo> players;
    int numOptional = 0;
    for (const auto &item : span) {
        players.emplace_back(item);
        if (!item.mandatory) numOptional++;
    }

    std::vector<CourtAllocation> result;

    for (int i = 0; i < numCourtAvailable && !players.empty(); i++) {
        BestCourtFinder finder = {numPlayersPerCourt_, stats_, minLevel_, maxLevel_, numOptional};
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
        numOptional -= finder.best->numOptional;
    }

    return result;
}
