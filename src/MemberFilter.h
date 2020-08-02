//
// Created by Fanchao Liu on 30/06/20.
//

#ifndef GAMEMATCHER_MEMBERFILTER_H
#define GAMEMATCHER_MEMBERFILTER_H

#include "models.h"
#include <variant>
#include <optional>

struct AllMembers {};
struct NonCheckedIn { SessionId sessionId; };
struct CheckedIn { SessionId sessionId; std::optional<bool> paused; };
struct AllSession { SessionId sessionId; };

typedef std::variant<AllMembers, NonCheckedIn, CheckedIn, AllSession> MemberSearchFilter;

static inline std::optional<SessionId> sessionIdFrom(const MemberSearchFilter &filter) {
    if (auto nci = std::get_if<NonCheckedIn>(&filter)) {
        return nci->sessionId;
    } else if (auto ci = std::get_if<CheckedIn>(&filter)) {
        return ci->sessionId;
    } else if (auto all = std::get_if<AllSession>(&filter)) {
        return all->sessionId;
    }

    return std::nullopt;
}


#endif //GAMEMATCHER_MEMBERFILTER_H
