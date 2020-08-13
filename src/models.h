#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QMetaEnum>
#include <QDateTime>
#include <QtDebug>

#define DECLARE_PROPERTY(type, name, defaultValue) \
    type name defaultValue; \
    Q_PROPERTY(type name MEMBER name);

typedef qlonglong MemberId;
typedef qlonglong SessionId;
typedef qlonglong CourtId;
typedef qlonglong GameId;
typedef QString SettingKey;


struct BaseMember {
Q_GADGET
private:
    mutable QString fullNameCache;
public:
    enum Gender {
        Male, Female
    };

    Q_ENUM(Gender);

    DECLARE_PROPERTY(MemberId, id, = 0);
    DECLARE_PROPERTY(qlonglong, registerDate, = 0);
    DECLARE_PROPERTY(QString, firstName,);
    DECLARE_PROPERTY(QString, lastName,);
    DECLARE_PROPERTY(Gender, gender, = Gender::Male);
    DECLARE_PROPERTY(int, level, = 0);
    DECLARE_PROPERTY(QString, phone,);
    DECLARE_PROPERTY(QString, email,);

    QString genderString() const {
        return QLatin1String(QMetaEnum::fromType<Gender>().valueToKey(gender));
    }

    QString fullName() const {
        if (fullNameCache.isNull()) {
            fullNameCache = QObject::tr("%1 %2", "full name").arg(firstName, lastName);
        }
        return fullNameCache;
    }

    inline bool operator<(const BaseMember &rhs) const {
        return id < rhs.id;
    }

    bool operator==(const BaseMember &rhs) const {
        return id == rhs.id &&
               registerDate == rhs.registerDate &&
               firstName == rhs.firstName &&
               lastName == rhs.lastName &&
               gender == rhs.gender &&
               level == rhs.level &&
               phone == rhs.phone &&
               email == rhs.email;
    }

    bool operator!=(const BaseMember &rhs) const {
        return !(rhs == *this);
    }
};

struct Member : BaseMember {
Q_GADGET
public:
    enum Status {
        Unknown, NotCheckedIn, CheckedIn, CheckedInPaused, CheckedOut
    };

    Q_ENUM(Status);

    DECLARE_PROPERTY(Status, status, = Unknown);
    DECLARE_PROPERTY(QVariant, paid,);

    bool operator==(const Member &rhs) const {
        return static_cast<const BaseMember &>(*this) == static_cast<const BaseMember &>(rhs) &&
               status == rhs.status &&
               paid.toBool() == rhs.paid.toBool();
    }

    bool operator!=(const Member &rhs) const {
        return !(rhs == *this);
    }

    QString displayName;
};


struct Session {
Q_GADGET
public:
    DECLARE_PROPERTY(SessionId, id, = 0);
    DECLARE_PROPERTY(int, fee, = 0);
    DECLARE_PROPERTY(QString, announcement,);
    DECLARE_PROPERTY(QString, place,);
    DECLARE_PROPERTY(QDateTime, startTime,);
    DECLARE_PROPERTY(int, numPlayersPerCourt, = 0);

    bool operator==(const Session &rhs) const {
        return id == rhs.id &&
               fee == rhs.fee &&
               announcement == rhs.announcement &&
               place == rhs.place &&
               startTime == rhs.startTime &&
               numPlayersPerCourt == rhs.numPlayersPerCourt;
    }

    bool operator!=(const Session &rhs) const {
        return !(rhs == *this);
    }
};


struct Court {
Q_GADGET
public:
    DECLARE_PROPERTY(CourtId, id, = 0);
    DECLARE_PROPERTY(SessionId, sessionId, = 0);
    DECLARE_PROPERTY(QString, name,);
    DECLARE_PROPERTY(int, sortOrder, = 0);

    bool operator==(const Court &rhs) const {
        return id == rhs.id &&
               sessionId == rhs.sessionId &&
               name == rhs.name &&
               sortOrder == rhs.sortOrder;
    }

    bool operator!=(const Court &rhs) const {
        return !(rhs == *this);
    }
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

    inline bool operator<(const GameAllocation &rhs) const {
        return gameId < rhs.gameId ||
                courtId < rhs.courtId ||
                memberId < rhs.memberId ||
                quality < rhs.quality;
    }

    bool operator==(const GameAllocation &rhs) const {
        return gameId == rhs.gameId &&
               courtId == rhs.courtId &&
               memberId == rhs.memberId &&
               quality == rhs.quality;
    }

    bool operator!=(const GameAllocation &rhs) const {
        return !(rhs == *this);
    }
};

inline static QDebug operator<<(QDebug dbg, const GameAllocation &c) {
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "("
                  << "gameId=" << c.gameId
                  << ",courtId=" << c.courtId
                  << ",memberId=" << c.memberId
                  << ")";

    return dbg;
}

void registerModels();

static inline auto DATE_TIME_FORMAT = QStringLiteral("hh:mma dd MMM");

#endif // MODELS_H
