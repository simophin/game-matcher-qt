//
// Created by Fanchao Liu on 1/07/20.
//

#ifndef GAMEMATCHER_TYPEUTILS_H
#define GAMEMATCHER_TYPEUTILS_H


#include <type_traits>

template<typename T, typename U = int>
struct HasMetaObject : std::false_type {};

template<typename T>
struct HasMetaObject<T, decltype((void) T::staticMetaObject, 0)> : std::true_type {};

#endif //GAMEMATCHER_TYPEUTILS_H
