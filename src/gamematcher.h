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
            QObject *parent = nullptr);
    ~GameMatcher();

signals:
    void onFinished(QVector<GameAllocation>);

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEMATCHER_H
