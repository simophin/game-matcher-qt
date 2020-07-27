#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include <QObject>
#include <optional>

#include "models.h"
#include "MemberFilter.h"
#include "span.h"

class QFile;
class QSqlDatabase;

struct CourtConfiguration {
    QString name;
    int sortOrder;
};

struct SessionData {
    Session session;
    QVector<Court> courts;
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

class ClubRepository : public QObject {
Q_OBJECT
public:
    static ClubRepository *open(QObject *parent, const QString &path);

    ~ClubRepository() override;

    QString getClubName() const;
    bool saveClubName(const QString&);

    std::pair<unsigned int, unsigned int> getLevelRange() const;

    bool saveClubInfo(const QString &name, unsigned int levelMin, unsigned int levelMax);

    std::optional<GameInfo> getLastGameInfo(SessionId) const;

    bool withdrawLastGame(SessionId);

    std::optional<QString> getSetting(const SettingKey &key) const;

    template<typename T>
    std::optional<T> getSettingValue(const SettingKey &k) const {
        auto v = getSetting(k);
        if (!v || v->isNull()) return std::nullopt;

        QVariant variant = *v;
        static auto typeId = qMetaTypeId<T>();
        if (variant.convert(typeId)) {
            return variant.value<T>();
        }

        return std::nullopt;
    }

    bool saveSettings(const SettingKey &key, const QVariant &value);

    std::optional<Member> createMember(
            const QString &fistName, const QString &lastName,
            const Member::Gender &gender, int level);

    bool saveMember(const Member &);

    QVector<Member> findMember(MemberSearchFilter, const QString &needle) const;

    std::optional<MemberId> findMemberBy(const QString &firstName, const QString &lastName);

    std::optional<Member> getMember(MemberId) const;

    QVector<Member> getMembers(MemberSearchFilter) const;

    bool checkIn(MemberId, SessionId, bool paid);

    bool checkOut(SessionId, MemberId);

    bool setPaused(SessionId, MemberId, bool);

    bool setPaid(SessionId, MemberId, bool);

    std::optional<SessionData> getSession(SessionId) const;

    std::optional<SessionId> getLastSession() const;

    std::optional<SessionData>
    createSession(int fee, const QString &place, const QString &announcement, int numPlayersPerCourt, const QVector<CourtConfiguration> &);

    QVector<GameAllocation> getPastAllocations(SessionId, std::optional<size_t> numGames = std::nullopt) const;

    std::optional<GameId> createGame(SessionId, nonstd::span<const GameAllocation>, qlonglong durationSeconds);

signals:

    void clubInfoChanged();
    void sessionChanged(SessionId);

private:
    ClubRepository(QObject *parent, const QSqlDatabase &);

    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
