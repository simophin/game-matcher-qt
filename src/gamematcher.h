#ifndef GAMEMATCHER_H
#define GAMEMATCHER_H

#include <QObject>
#include <QVector>

#include "models.h"

class GameMatcher : public QObject
{
    Q_OBJECT
public:
    explicit GameMatcher(const QVector<GameAllocation> &pastAllocation,
                         const QVector<Member> &members,
                         const QVector<Player> &players,
                         const QVector<CourtId> &courts,
                         int playerPerCourt,
                         int seed,
                         QObject *parent = nullptr);
    ~GameMatcher() override;

signals:
    void onFinished(QVector<GameAllocation>);

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEMATCHER_H
