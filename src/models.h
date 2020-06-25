#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QDateTime>
#include <QtDebug>

#define DECLARE_PROPERTY(type, name, defaultValue) \
    type name defaultValue; \
    Q_PROPERTY(type name MEMBER name)

typedef int MemberId;
typedef int PlayerId;
typedef int SessionId;
typedef int CourtId;
typedef int GameId;


struct Setting {
    Q_GADGET
public:
    DECLARE_PROPERTY(QString, name, );
    DECLARE_PROPERTY(QString, value, );
};


struct Member {
Q_GADGET
public:

    DECLARE_PROPERTY(MemberId, id, = 0);
    DECLARE_PROPERTY(QDateTime, registerDate,);
    DECLARE_PROPERTY(QString, firstName,);
    DECLARE_PROPERTY(QString, lastName,);
    DECLARE_PROPERTY(QString, gender,);
    DECLARE_PROPERTY(int, level, = 0);
    DECLARE_PROPERTY(int, initialBalance, = 0);
};


struct Session {
    Q_GADGET
public:
    DECLARE_PROPERTY(SessionId, id, = 0);
    DECLARE_PROPERTY(int, fee, = 0);
    DECLARE_PROPERTY(QString, announcement, );
    DECLARE_PROPERTY(QDateTime, startTime, );
};


struct Player {
Q_GADGET
public:

    DECLARE_PROPERTY(PlayerId, id, = 0);
    DECLARE_PROPERTY(SessionId, sessionId, = 0);
    DECLARE_PROPERTY(MemberId, memberId, = 0);
    DECLARE_PROPERTY(QDateTime, checkInTime, );
    DECLARE_PROPERTY(QDateTime, checkOutTime, );
    DECLARE_PROPERTY(int, payment, = 0);
    DECLARE_PROPERTY(bool, paused, = false);
};


struct Court {
Q_GADGET
public:

    DECLARE_PROPERTY(CourtId, id, = 0);
    DECLARE_PROPERTY(SessionId, sessionId, = 0);
    DECLARE_PROPERTY(QString, name, );
    DECLARE_PROPERTY(int, sortOrder, = 0);
};


struct GameAllocation {
Q_GADGET
public:
    DECLARE_PROPERTY(GameId, gameId, = 0);
    DECLARE_PROPERTY(CourtId, courtId, = 0);
    DECLARE_PROPERTY(PlayerId, playerId, = 0);

    GameAllocation() = default;

    GameAllocation(GameId gameId, CourtId courtId, PlayerId playerId)
            : gameId(gameId), courtId(courtId), playerId(playerId) {}
};

inline static QDebug operator<<(QDebug dbg, const GameAllocation &c)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "("
        << "gameId=" << c.gameId
        << ",courtId=" << c.courtId
        << ",playerId=" << c.playerId
        << ")";

    return dbg;
}

#endif // MODELS_H
