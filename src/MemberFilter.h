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

typedef std::variant<AllMembers, NonCheckedIn, CheckedIn> MemberSearchFilter;


#endif //GAMEMATCHER_MEMBERFILTER_H
