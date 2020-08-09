#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include "models.h"

#include "span.h"
#include <vector>

class GameMatcher {
public:
    static std::vector<GameAllocation>
    match(nonstd::span<const GameAllocation> pastAllocation, nonstd::span<const Member> members, nonstd::span<const CourtId> courts,
          size_t playerPerCourt, int seed);
};

#endif // GAMEMATCHER_H
