//
// Created by Fanchao Liu on 20/08/20.
//

#ifndef GAMEMATCHER_MOCKGAMESTATS_H
#define GAMEMATCHER_MOCKGAMESTATS_H

#include "GameStats.h"

#include <map>
#include <functional>

struct MockGameStats : public GameStats {
    std::map<MemberId, int> numGamesByMember;
    std::map<MemberId, int> numGamesOffByMember;
    int totalGame = 0;
    std::function<bool(const QVector<MemberId>&)> scorer;

    int numGamesFor(MemberId id) const override {
        auto found = numGamesByMember.find(id);
        if (found != numGamesByMember.end()) return found->second;
        return 0;
    }

    int numGamesOff(MemberId id) const override {
        auto found = numGamesOffByMember.find(id);
        if (found != numGamesOffByMember.end()) return found->second;
        return 0;
    }

    int numGames() const override {
        return totalGame;
    }

    int similarityScore(const QVector<MemberId> &vector) const override {
        return scorer(vector);
    }
};

#endif //GAMEMATCHER_MOCKGAMESTATS_H
