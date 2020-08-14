//
// Created by Fanchao Liu on 16/07/20.
//

#ifndef GAMEMATCHER_PLAYERINFO_H
#define GAMEMATCHER_PLAYERINFO_H

#include "models.h"

#include <optional>
#include <QtDebug>

struct BasePlayerInfo {
    MemberId memberId;
    BaseMember::Gender gender;
    int level;

    inline BasePlayerInfo(MemberId id, BaseMember::Gender gender, int level)
            : memberId(id), gender(gender), level(level) {}

    inline explicit BasePlayerInfo(const Member &m) : BasePlayerInfo(m.id, m.gender, m.level) {}
};

struct PlayerInfo : BasePlayerInfo {
    bool mandatory;

    inline PlayerInfo(const Member &m, bool mandatory)
            : BasePlayerInfo(m), mandatory(mandatory) {}

    inline PlayerInfo(MemberId memberId, Member::Gender gender, int level, bool mandatory)
            : BasePlayerInfo(memberId, gender, level), mandatory(mandatory) {}

    inline PlayerInfo(const BasePlayerInfo &rhs, bool mandatory)
            : BasePlayerInfo(rhs), mandatory(mandatory) {}

    PlayerInfo(const PlayerInfo &) = default;
};


inline QDebug operator<<(QDebug debug, const PlayerInfo &p) {
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << p.memberId << '(' << p.level << ')';
    return debug;
}

#endif //GAMEMATCHER_PLAYERINFO_H
