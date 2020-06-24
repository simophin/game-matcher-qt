#include "gamematcher.h"

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>

#include <QtDebug>
#include <QHash>

#include <algorithm>

#include "gamestats.h"

struct GameMatcher::Impl {
};

struct PlayerInfo {
    Player player;
    Member member;
    int eligibilityScore = 0;
};

GameMatcher::GameMatcher(const QVector<GameAllocation> &pastAllocation,
                         const QVector<Member> &members,
                         const QVector<Player> &players,
                         QObject *parent) : QObject(parent), d(new Impl) {
    typedef QFutureWatcher<QVector<GameAllocation>> Watcher;
    auto watcher = new Watcher(this);
    watcher->setFuture(QtConcurrent::run([=] {
        GameStats stats(pastAllocation, members, players);

        auto memberById = std::reduce(
                members.constBegin(), members.constEnd(), QHash<MemberId, Member>(),
                [](auto &map, const auto &member) {
                    map[member.id] = member;
                    return map;
                });

        QVector<PlayerInfo> eligiblePlayers;
        for (const auto &player : players) {
            if (player.paused || player.checkOutTime.isValid()) continue;
            const auto member = memberById.find(player.memberId);
            if (member == memberById.constEnd()) continue;

            eligiblePlayers.append({
                player,
                *member,
                -stats.numGamesFor(player.id) + member->level
            });
        }
        std::sort(eligiblePlayers.begin(), eligiblePlayers.end(), [](const auto &a, const auto &b) {
            return a.eligibilityScore > b.eligibilityScore;
        });

        return QVector<GameAllocation>{GameAllocation()};
    }));

    connect(watcher, &Watcher::finished, [this, watcher] {
        emit onFinished(watcher->result());
        watcher->deleteLater();
    });
}

GameMatcher::~GameMatcher() {
    delete d;
}
