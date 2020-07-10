//
// Created by Fanchao Liu on 6/07/20.
//

#ifndef GAMEMATCHER_COLLECTIONUTILS_H
#define GAMEMATCHER_COLLECTIONUTILS_H

#include <optional>
#include <functional>

template<typename Map, typename Col, typename ByFunc>
static inline Map associateBy(const Col &collection, ByFunc keySelector) {
    Map m;
    for (const auto &item : collection) {
        m[keySelector(item)] = item;
    }
    return m;
}

template <typename Map>
static inline std::optional<typename Map::mapped_type> getMapValue(const Map &m, const typename Map::key_type &k) {
    if (auto found = m.find(k); found != m.end()) {
        return *found;
    }

    return std::nullopt;
}

template <typename Col, typename T, typename Reducer>
static inline T reduceCollection(const Col &col, T initialValue, Reducer reducer) {
    for (const auto & e : col) {
        initialValue = reducer(initialValue, e);
    }
    return initialValue;
}

#define sumBy(collection, fieldName) \
    reduceCollection(collection, 0, [](int sum, const auto &ele) { return sum + ele.fieldName; })

#endif //GAMEMATCHER_COLLECTIONUTILS_H
