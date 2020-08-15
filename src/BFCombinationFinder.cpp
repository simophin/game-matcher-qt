//
// Created by Fanchao Liu on 16/07/20.
//

#include "BFCombinationFinder.h"
#include "MatchingScore.h"

#include <list>
#include <optional>


typedef std::list<PlayerInfo> PlayerInfoList;
typedef typename PlayerInfoList::iterator PlayerInfoIterator;

struct BestCombination {
    std::vector<PlayerInfoIterator> players;
    int score = 0;
    unsigned numMandatory = 0;
};

class BestCourtFinder {
    unsigned const numPlayerRequired;
    GameStats const &stats;
    int const minLevel, maxLevel;

public:
    BestCourtFinder(const unsigned numPlayerRequired, const GameStats &stats, const int minLevel, const int maxLevel)
            : numPlayerRequired(numPlayerRequired), stats(stats), minLevel(minLevel), maxLevel(maxLevel) {}

private:

    PlayerInfoIterator inputEnd;
    unsigned minMandatoryRequired;
    unsigned numEstimated = 0;

    std::vector<PlayerInfoIterator> arrangement;
    unsigned numMandatory = 0;

public:
    std::optional<BestCombination> best;

    void reset(unsigned minMandatory, PlayerInfoIterator end) {
        inputEnd = end;
        minMandatoryRequired = minMandatory;
        numEstimated = numMandatory = 0;
        arrangement.clear();
        best.reset();
    }

    void find(PlayerInfoIterator begin) {
        if (arrangement.size() == numPlayerRequired) {
            if (numMandatory < minMandatoryRequired) {
                return;
            }

            numEstimated++;

            int score = MatchingScore::computeCourtScore(stats, arrangement, minLevel, maxLevel);
            bool update;
            if (!best) {
                best.emplace();
                update = true;
            } else {
                update = score > best->score;
            }

            if (update) {
                best->players.clear();
                best->players.insert(best->players.end(), arrangement.begin(), arrangement.end());
                best->score = score;
                best->numMandatory = numMandatory;
            }
        } else {
            while (begin != inputEnd) {
                bool isMandatory = begin->mandatory;
                arrangement.emplace_back(begin++);
                if (isMandatory) numMandatory++;

                find(begin);

                arrangement.pop_back();
                if (isMandatory) numMandatory--;
            }
        }
    }
};


QVector<CombinationFinder::CourtAllocation>
BFCombinationFinder::doFind(const QVector<PlayerInfo> &span, unsigned numCourtAvailable) const {
    if (span.empty()) return {};

    std::list<PlayerInfo> players;
    unsigned numMandatoryRequired = 0;
    std::optional<int> minLevel, maxLevel;
    for (const auto &item : span) {
        players.emplace_back(item);
        if (item.mandatory) {
            numMandatoryRequired++;
        }
        if (!minLevel || item.level < *minLevel) {
            minLevel = item.level;
        }

        if (!maxLevel || item.level > *maxLevel) {
            maxLevel = item.level;
        }
    }

    QVector<CourtAllocation> result;
    BestCourtFinder finder(numPlayersPerCourt_, stats_, *minLevel, *maxLevel);

    auto numCourtAllocated = std::min<unsigned>(numCourtAvailable, players.size() / numPlayersPerCourt_);

    for (int i = 0; i < numCourtAllocated; i++) {
        finder.reset(static_cast<unsigned>(std::ceil(
                static_cast<double>(numMandatoryRequired) / (numCourtAllocated - i))),
                     players.end());

        finder.find(players.begin());
        if (!finder.best) {
            qWarning() << "Unable to find best court";
            break;
        }

        result.push_back(CourtAllocation());
        auto &allocation = *result.rbegin();
        for (const auto &player : finder.best->players) {
            allocation.players.push_back(*player);
            players.erase(player);
        }
        allocation.quality = finder.best->score;
        numMandatoryRequired -= finder.best->numMandatory;
    }

    return result;
}
