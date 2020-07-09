//
// Created by Fanchao Liu on 9/07/20.
//

#ifndef GAMEMATCHER_COMBINATIONSFINDER_H
#define GAMEMATCHER_COMBINATIONSFINDER_H

#include "span.h"

#include <vector>
#include <optional>
#include <functional>


template<typename T>
class CombinationsFinder {
public:
    typedef std::function<int(nonstd::span<T>)> ScoreFunction;

    struct BestResult {
        std::vector<T> data;
        int score;
    };

    CombinationsFinder(const int numCombinationsRequired, ScoreFunction &&func) :
            numCombinationsRequired_(numCombinationsRequired), scoreFunction_(std::move(func)) {}

private:
    template<typename Iterator>
    void doFind(Iterator begin, Iterator end) {
        if (out_.size() == numCombinationsRequired_) {
            auto score = scoreFunction_(out_);
            if (!bestResult_) {
                bestResult_ = { out_, score };
            } else if (!bestResult_->score < score) {
                bestResult_->data.clear();
                bestResult_->data.insert(bestResult_->end(), out_.begin(), out_.end());
            }
        }
    }

    int const numCombinationsRequired_;
    ScoreFunction const scoreFunction_;
    std::vector<T> out_;

    std::optional<BestResult> bestResult_;
};


#endif //GAMEMATCHER_COMBINATIONSFINDER_H
