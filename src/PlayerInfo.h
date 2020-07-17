//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_PLAYERINFO_H
#define GAMEMATCHER_PLAYERINFO_H

#include "models.h"

#include <optional>
#include <QtDebug>

struct PlayerInfo {
    MemberId memberId;
    Member::Gender gender;
    int level;
    std::optional<int> eligibilityScore;

    inline bool optionalOn() const { return !eligibilityScore.has_value(); }

    inline PlayerInfo(const Member &m, int score)
            :memberId(m.id), gender(m.gender), level(m.level), eligibilityScore(score) {}
};


inline QDebug operator<<(QDebug debug, const PlayerInfo &p) {
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << p.memberId << '(' << p.level << ')';
    return debug;
}

#endif //GAMEMATCHER_PLAYERINFO_H
