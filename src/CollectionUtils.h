//
// Created by Fanchao Liu on 6/07/20.
//

#ifndef GAMEMATCHER_COLLECTIONUTILS_H
#define GAMEMATCHER_COLLECTIONUTILS_H

#include <optional>
#include <functional>

#include <QtDebug>

#include "span.h"

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
    for (auto & e : col) {
        initialValue = reducer(initialValue, e);
    }
    return initialValue;
}

#define sumBy(collection, fieldName) \
    reduceCollection(collection, 0, [](int sum, const auto &ele) { return sum + ele.fieldName; })

template <typename T>
inline QDebug operator<<(QDebug debug, nonstd::span<T> c)
{
    QDebugStateSaver saver(debug);

    debug.noquote().nospace() << '[';
    for (size_t i = 0, size = c.size(); i < size; i++) {
        debug << c[i];
        if (i < size - 1) debug << ",";
    }
    debug << ']';
    return debug;
}

#endif //GAMEMATCHER_COLLECTIONUTILS_H
