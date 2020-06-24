#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QDateTime>

#define DECLARE_PROPERTY(type, name) \
    type name; \
    Q_PROPERTY(type name MEMBER name)

typedef int MemberId;
typedef int PlayerId;
typedef int SessionId;
typedef int CourtId;
typedef int GameId;

struct Member {
Q_GADGET
public:

    DECLARE_PROPERTY(MemberId, id);
    DECLARE_PROPERTY(QDateTime, registerDate);
    DECLARE_PROPERTY(QString, fistName);
    DECLARE_PROPERTY(QString, lastName);
    DECLARE_PROPERTY(QString, gender);
    DECLARE_PROPERTY(int, level);
    DECLARE_PROPERTY(int, initialBalance);
};

Q_DECLARE_METATYPE(Member);

struct Player {
Q_GADGET
public:

    DECLARE_PROPERTY(PlayerId, id);
    DECLARE_PROPERTY(SessionId, sessionId);
    DECLARE_PROPERTY(MemberId, memberId);
    DECLARE_PROPERTY(QDateTime, checkInTime);
    DECLARE_PROPERTY(QDateTime, checkOutTime);
    DECLARE_PROPERTY(bool, paused);
};

Q_DECLARE_METATYPE(Player);

struct Court {
Q_GADGET
public:

    DECLARE_PROPERTY(CourtId, id);
    DECLARE_PROPERTY(SessionId, sessionId);
    DECLARE_PROPERTY(QString, name);
    DECLARE_PROPERTY(int, sortOrder);
};

Q_DECLARE_METATYPE(Court);

struct GameAllocation {
Q_GADGET
public:
    DECLARE_PROPERTY(GameId, gameId);
    DECLARE_PROPERTY(CourtId, courtId);
    DECLARE_PROPERTY(PlayerId, playerId);

    GameAllocation() = default;

    GameAllocation(GameId gameId, CourtId courtId, PlayerId playerId)
            : gameId(gameId), courtId(courtId), playerId(playerId) {}
};

Q_DECLARE_METATYPE(GameAllocation);

#endif // MODELS_H
