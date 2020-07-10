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

    struct BestResult {
        std::vector<T> data;
        int score;
    };

    CombinationsFinder(const int numCombinationsRequired) :
            numCombinationsRequired_(numCombinationsRequired) {}

    template<typename Iterator>
    const BestResult* find(Iterator begin, Iterator end) {
        bestResult_.reset();
        out_.clear();
        doFind(begin, end);
        if (bestResult_) return &bestResult_.value();
        return nullptr;
    }

protected:
    virtual std::optional<int> computeScore(nonstd::span<T>) = 0;

private:
    template<typename Iterator>
    void doFind(Iterator begin, Iterator end) {
        if (out_.size() == numCombinationsRequired_ || begin == end) {
            auto score = computeScore(out_);
            if (!score) return;

            if (!bestResult_) {
                bestResult_ = { out_, *score };
            } else if (bestResult_->score < *score) {
                bestResult_->data.clear();
                bestResult_->data.insert(bestResult_->data.end(), out_.begin(), out_.end());
                bestResult_->score = *score;
            }
        } else {
            for (auto iter = begin; iter != end;) {
                out_.push_back(*iter);
                doFind(++iter, end);
                out_.pop_back();
            }
        }
    }

    int const numCombinationsRequired_;
    std::vector<T> out_;

    std::optional<BestResult> bestResult_;
};


#endif //GAMEMATCHER_COMBINATIONSFINDER_H
