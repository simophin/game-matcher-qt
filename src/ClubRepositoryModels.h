//
// Created by Fanchao Liu on 5/08/20.
//

#ifndef GAMEMATCHER_CLUBREPOSITORYMODELS_H
#define GAMEMATCHER_CLUBREPOSITORYMODELS_H

#include "models.h"

struct CourtConfiguration {
    QString name;
    int sortOrder;
};

struct SessionData {
    Session session;
    QVector<Court> courts;

    bool operator==(const SessionData &rhs) const {
        return session == rhs.session &&
               courts == rhs.courts;
    }

    bool operator!=(const SessionData &rhs) const {
        return !(rhs == *this);
    }
};

struct CourtPlayers {
    Q_GADGET
public:
    DECLARE_PROPERTY(CourtId, courtId,);
    DECLARE_PROPERTY(QString, courtName,);
    DECLARE_PROPERTY(QVector<Member>, players,);

    bool operator==(const CourtPlayers &rhs) const {
        return courtId == rhs.courtId &&
               courtName == rhs.courtName &&
               players == rhs.players;
    }

    bool operator!=(const CourtPlayers &rhs) const {
        return !(rhs == *this);
    }
};

Q_DECLARE_METATYPE(CourtPlayers);

struct GameInfo {
    Q_GADGET
public:
    DECLARE_PROPERTY(GameId, id,);
    DECLARE_PROPERTY(qlonglong, startTime,);
    DECLARE_PROPERTY(qlonglong, durationSeconds,);
    DECLARE_PROPERTY(QVector<CourtPlayers>, courts,);
    DECLARE_PROPERTY(QVector<Member>, waiting,);

    QDateTime startDateTime() const {
        return QDateTime::fromSecsSinceEpoch(startTime);
    }
};

struct MemberGameStats {
    struct PastGame {
        GameId gameId;
        GameId courtId;
        QString courtName;
        QDateTime startTime;
        int quality;
        QVector<BaseMember> players;
    };

    size_t numGamesOff = 0;
    QVector<PastGame> pastGames;
};

struct LevelRange {
    Q_GADGET
public:
    DECLARE_PROPERTY(unsigned, min, = 0);
    DECLARE_PROPERTY(unsigned, max, = 0);

    bool operator==(const LevelRange &rhs) const {
        return min == rhs.min &&
               max == rhs.max;
    }

    bool operator!=(const LevelRange &rhs) const {
        return !(rhs == *this);
    }

    bool isValid() const {
        return min <= max;
    }
};

#endif //GAMEMATCHER_CLUBREPOSITORYMODELS_H
