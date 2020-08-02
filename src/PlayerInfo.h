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
    bool mandatory;

    inline PlayerInfo(const Member &m, bool mandatory)
            : memberId(m.id), gender(m.gender), level(m.level), mandatory(mandatory) {}

    PlayerInfo(MemberId memberId, Member::Gender gender, int level, bool mandatory)
            : memberId(memberId),
              gender(gender), level(level),
              mandatory(mandatory) {}

    PlayerInfo(const PlayerInfo&) = default;
};


inline QDebug operator<<(QDebug debug, const PlayerInfo &p) {
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << p.memberId << '(' << p.level << ')';
    return debug;
}

#endif //GAMEMATCHER_PLAYERINFO_H
