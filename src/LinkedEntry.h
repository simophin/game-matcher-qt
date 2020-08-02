//
// Created by Fanchao Liu on 2/08/20.
//

#ifndef GAMEMATCHER_LINKEDENTRY_H
#define GAMEMATCHER_LINKEDENTRY_H

#include <type_traits>
#include <assert.h>
#include <utility>

template <typename T>
struct LinkedEntry {
    T *prev = nullptr, *next = nullptr;

    void remove() {
        assert(next || prev);
        if (prev) prev->next = next;
        if (next) next->prev = prev;
        prev = next = nullptr;
    }

    void insertAfter(T *obj) {
        assert(!obj->next && !obj->prev);
        if (next) next->prev = obj;
        obj->next = next;
        next = obj;
        obj->prev = static_cast<T *>(this);
    }

    void insertBefore(T *obj) {
        assert(!obj->next && !obj->prev);
        if (prev) prev->next = obj;
        obj->prev = prev;
        prev = obj;
        obj->next = static_cast<T *>(this);
    }

    bool isTail() const {
        assert(prev || next);
        return !next;
    }

    bool isHead() const {
        assert(prev || next);
        return !prev;
    }
};

#endif //GAMEMATCHER_LINKEDENTRY_H
