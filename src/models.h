#ifndef MODELS_H
#define MODELS_H

#include <QObject>
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


static const auto genderMale = QStringLiteral("male");
static const auto genderFemale = QStringLiteral("female");

static const auto levelMin = 0;
static const auto levelMax = 10;

struct Member {
Q_GADGET
public:
    DECLARE_PROPERTY(MemberId, id, = 0);
    DECLARE_PROPERTY(QDateTime, registerDate,);
    DECLARE_PROPERTY(QString, firstName,);
    DECLARE_PROPERTY(QString, lastName,);
    DECLARE_PROPERTY(QString, gender,);
    DECLARE_PROPERTY(QString, email,);
    DECLARE_PROPERTY(QString, phone,);
    DECLARE_PROPERTY(int, level, = 0);

    QString displayName;

    QString fullName() const {
        return QObject::tr("%1 %2 (%3, %4)", "full name")
            .arg(firstName, lastName, QString::number(level), gender.left(1));
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

    GameAllocation() = default;

    GameAllocation(GameId gameId, CourtId courtId, MemberId memberId)
            : gameId(gameId), courtId(courtId), memberId(memberId) {}
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
