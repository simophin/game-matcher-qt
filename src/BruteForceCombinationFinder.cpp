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
    PlayerInfoIterator const end;
    int const numRequired;
    GameStats const &stats;
    int const minLevel, maxLevel;
    int const numMaxOptional;

    std::vector<PlayerInfoIterator> players;
    int numOptional = 0;

    std::optional<BestCombination> best;

    void find(PlayerInfoIterator begin) {
        if (players.size() == numRequired) {
            if (numOptional > numMaxOptional) {
                return;
            }

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
        } else  {
            while (begin != end) {
                bool isOptional = begin->optionalOn();
                players.emplace_back(begin++);
                if (isOptional) numOptional++;
                find(begin);
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
        if (item.optionalOn()) numOptional++;
    }

    int i = 0;
    while (!players.end())

    return std::vector<CourtAllocation>();
}
