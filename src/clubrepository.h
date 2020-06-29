#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include <QObject>
#include <optional>
#include <qqml.h>

#include "models.h"

class QFile;

struct CourtConfiguration {
    QString name;
    int sortOrder;
};

struct SessionData {
    Session session;
    QVector<Member> checkedIn;
    QVector<Court> courts;
};

struct MemberInfo : Member {
Q_GADGET
    QML_ELEMENT
public:
    DECLARE_PROPERTY(int, balance, = 0);
};

struct MemberSearchResult : Member {
Q_GADGET
public:
    DECLARE_PROPERTY(QString, matched,);
};

struct ClubInfo {
Q_GADGET
    QML_ELEMENT
public:
    DECLARE_PROPERTY(bool, valid, = false);
    DECLARE_PROPERTY(QString, name,);
    DECLARE_PROPERTY(int, sessionFee,);
    DECLARE_PROPERTY(QDateTime, creationDate,);
};

struct CourtPlayers {
Q_GADGET
    QML_ELEMENT
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

struct GameInfo {
Q_GADGET
    QML_ELEMENT
public:
    DECLARE_PROPERTY(QString, announcement,);
    DECLARE_PROPERTY(QDateTime, startTime,);
    DECLARE_PROPERTY(QVector<CourtPlayers>, courts,);

    Q_INVOKABLE bool empty() const {
        return startTime.isNull() || courts.empty();
    }
};

class ClubRepository : public QObject {
Q_OBJECT
    QML_ELEMENT
public:
    explicit ClubRepository(QObject *parent = nullptr);

    ~ClubRepository() override;

    bool open(const QString &);

    Q_INVOKABLE void close();

    bool isValid() const;

    Q_PROPERTY(bool isValid READ isValid);

    ClubInfo clubInfo() const;

    bool setClubInfo(const ClubInfo &);

    Q_PROPERTY(ClubInfo clubInfo READ clubInfo WRITE setClubInfo NOTIFY clubInfoChanged);

    GameInfo lastGameInfo() const;

    Q_PROPERTY(GameInfo lastGameInfo READ lastGameInfo NOTIFY lastGameInfoChanged STORED false);

    QString getSettings(const QString &key) const;

    bool setSettings(const QString &key, const QVariant &value);

    std::optional<MemberInfo> createMember(
            const QString &fistName, const QString &lastName,
            const QString &gender, int level);

    [[nodiscard]] QVector<MemberSearchResult> findMember(const QString &needle) const;

    std::optional<MemberInfo> getMember(MemberId) const;

    std::optional<Player> getPlayer(PlayerId) const;

    std::optional<Player> checkIn(MemberId, SessionId, int payment) const;

    bool checkOut(PlayerId);


    std::optional<SessionData> getSession(SessionId) const;

    std::optional<SessionId> getLastSession() const;

    std::optional<SessionData> createSession(int fee, const QString &announcement, const QVector<CourtConfiguration> &);

    [[nodiscard]] QVector<GameAllocation> getPastAllocations(SessionId) const;

    [[nodiscard]] QVector<GameAllocation> getLastGameAllocation(SessionId) const;

    std::optional<GameId> createGame(SessionId, const QVector<GameAllocation> &);

signals:

    void clubInfoChanged();

    void lastGameInfoChanged();

    void lastSessionChanged();

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
