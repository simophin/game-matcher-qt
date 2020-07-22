#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QMetaEnum>
#include <QDateTime>
#include <QtDebug>

#define DECLARE_PROPERTY(type, name, defaultValue) \
    type name defaultValue; \
    Q_PROPERTY(type name MEMBER name)

typedef qlonglong MemberId;
typedef qlonglong SessionId;
typedef qlonglong CourtId;
typedef qlonglong GameId;
typedef QString SettingKey;

struct Setting {
    Q_GADGET
public:
    DECLARE_PROPERTY(SettingKey, name, );
    DECLARE_PROPERTY(QString, value, );
};


struct Member {
Q_GADGET
private:
    mutable QString fullNameCache;
public:
    enum Gender {
        Male, Female
    };
    Q_ENUM(Gender);

    DECLARE_PROPERTY(MemberId, id, = 0);
    DECLARE_PROPERTY(QDateTime, registerDate,);
    DECLARE_PROPERTY(QString, firstName,);
    DECLARE_PROPERTY(QString, lastName,);
    DECLARE_PROPERTY(Gender, gender, = Gender::Male);
    DECLARE_PROPERTY(QString, email,);
    DECLARE_PROPERTY(QString, phone,);
    DECLARE_PROPERTY(int, level, = 0);

    QString genderString() const {
        return QLatin1String(QMetaEnum::fromType<Gender>().valueToKey(gender));
    }

    enum Status {
        NotCheckedIn, CheckedIn, CheckedInPaused, CheckedOut
    };
    Q_ENUM(Status);

    DECLARE_PROPERTY(QVariant, status, );
    DECLARE_PROPERTY(QVariant, paid, );

    QString displayName;

    QString fullName() const {
        if (fullNameCache.isNull()) {
            fullNameCache = QObject::tr("%1 %2", "full name").arg(firstName, lastName);
        }
        return fullNameCache;
    }

    bool operator==(const Member &rhs) const {
        return id == rhs.id &&
               registerDate == rhs.registerDate &&
               firstName == rhs.firstName &&
               lastName == rhs.lastName &&
               gender == rhs.gender &&
               level == rhs.level;
    }

    bool operator!=(const Member &rhs) const {
        return !(rhs == *this);
    }
};


struct Session {
    Q_GADGET
public:
    DECLARE_PROPERTY(SessionId, id, = 0);
    DECLARE_PROPERTY(int, fee, = 0);
    DECLARE_PROPERTY(QString, announcement, );
    DECLARE_PROPERTY(QString, place, );
    DECLARE_PROPERTY(QDateTime, startTime, );
    DECLARE_PROPERTY(int, numPlayersPerCourt, = 0);
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
    DECLARE_PROPERTY(MemberId, memberId, = 0);
    DECLARE_PROPERTY(int, quality, = 0);

    GameAllocation() = default;

    GameAllocation(GameId gameId, CourtId courtId, MemberId memberId, int quality)
            : gameId(gameId), courtId(courtId), memberId(memberId), quality(quality) {}
};

inline static QDebug operator<<(QDebug dbg, const GameAllocation &c)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "("
        << "gameId=" << c.gameId
        << ",courtId=" << c.courtId
        << ",memberId=" << c.memberId
        << ")";

    return dbg;
}

#endif // MODELS_H
