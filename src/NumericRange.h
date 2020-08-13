//
// Created by Fanchao Liu on 12/08/20.
//

#ifndef GAMEMATCHER_NUMERICRANGE_H
#define GAMEMATCHER_NUMERICRANGE_H

#include <optional>
#include <cstddef>

template<typename T>
struct NumericRange {
    std::optional<T> min, max;

    inline NumericRange(const std::optional<T> &min, const std::optional<T> &max) : min(min), max(max) {
        if (min && max) {
            assert(*max >= *min);
        }
    }
};

typedef NumericRange<size_t> SizeRange;

#endif //GAMEMATCHER_NUMERICRANGE_H
