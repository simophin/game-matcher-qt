#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include <QObject>
#include <QVector>

#include "models.h"

class GameMatcher : public QObject {
Q_OBJECT
public:
    explicit GameMatcher(QObject *parent = nullptr): QObject(parent) {}

public slots:
    QVector<GameAllocation> match(
            const QVector<GameAllocation> &pastAllocation,
            const QVector<Member> &members,
            const QVector<Player> &players,
            const QVector<CourtId> &courts,
            int playerPerCourt,
            int seed) const;
};

#endif // GAMEMATCHER_H
