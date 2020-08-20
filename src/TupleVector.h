//
// Created by Fanchao Liu on 20/08/20.
//

#ifndef GAMEMATCHER_TUPLEVECTOR_H
#define GAMEMATCHER_TUPLEVECTOR_H

#include <QVector>

#include <tuple>

template<typename...Args>
class TupleVector : public QVector<std::tuple<Args...>> {
public:
    using QVector<std::tuple<Args...>>::QVector;
};

#endif //GAMEMATCHER_TUPLEVECTOR_H
