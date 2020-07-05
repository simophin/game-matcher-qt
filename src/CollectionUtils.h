//
// Created by Fanchao Liu on 6/07/20.
//

#ifndef GAMEMATCHER_COLLECTIONUTILS_H
#define GAMEMATCHER_COLLECTIONUTILS_H


template<typename Map, typename Col, typename ByFunc>
static inline Map associateBy(const Col &collection, ByFunc keySelector) {
    Map m;
    for (const auto &item : collection) {
        m[keySelector(item)] = item;
    }
    return m;
}

#endif //GAMEMATCHER_COLLECTIONUTILS_H
