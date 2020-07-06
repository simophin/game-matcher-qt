#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include <QObject>
#include <optional>

#include "models.h"
#include "MemberFilter.h"

class QFile;

struct CourtConfiguration {
    QString name;
    int sortOrder;
};

struct SessionData {
    Session session;
    QVector<Court> courts;
};


struct ClubInfo {
Q_GADGET
public:
    DECLARE_PROPERTY(QString, name,);
    DECLARE_PROPERTY(QDateTime, creationDate,);
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
    DECLARE_PROPERTY(QDateTime, startTime,);
    DECLARE_PROPERTY(QVector<CourtPlayers>, courts,);

    Q_INVOKABLE bool empty() const {
        return startTime.isNull() || courts.empty();
    }
};

class ClubRepository : public QObject {
Q_OBJECT
public:
    explicit ClubRepository(QObject *parent = nullptr);

    ~ClubRepository() override;

    bool open(const QString &);

    Q_INVOKABLE void close();

    bool isValid() const;

    Q_PROPERTY(bool isValid READ isValid);

    ClubInfo getClubInfo() const;

    bool saveClubInfo(const ClubInfo &);

    std::optional<GameInfo> getLastGameInfo(SessionId) const;

    QString getSetting(const SettingKey &key) const;

    bool saveSettings(const SettingKey &key, const QVariant &value);

    std::optional<Member> createMember(
            const QString &fistName, const QString &lastName,
            const QString &gender, int level);

    bool saveMember(const Member &);

    QVector<Member> findMember(MemberSearchFilter, const QString &needle) const;

    std::optional<MemberId> findMemberBy(const QString &firstName, const QString &lastName);

    std::optional<Member> getMember(MemberId) const;

    QVector<Member> getMembers(MemberSearchFilter) const;

    bool checkIn(MemberId, SessionId, bool paid);

    bool checkOut(SessionId, MemberId);

    bool setPaused(SessionId, MemberId, bool);

    std::optional<SessionData> getSession(SessionId) const;

    std::optional<SessionId> getLastSession() const;

    std::optional<SessionData>
    createSession(int fee, const QString &place, const QString &announcement, int numPlayersPerCourt, const QVector<CourtConfiguration> &);

    QVector<GameAllocation> getPastAllocations(SessionId, std::optional<size_t> numGames = std::nullopt) const;

    std::optional<GameId> createGame(SessionId, const QVector<GameAllocation> &);

signals:

    void clubInfoChanged();
    void sessionChanged(SessionId);

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
