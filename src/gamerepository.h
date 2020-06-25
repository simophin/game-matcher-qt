#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include <QObject>
#include <optional>

#include "models.h"

class QFile;

struct CourtConfiguration {
    QString name;
    int sortOrder;
};

struct SessionData {
    Session session;
    QVector<Court> courts;
};

struct MemberInfo : Member {
    Q_GADGET
public:
    DECLARE_PROPERTY(int, balance, = 0);
};

struct MemberSearchResult : Member {
    Q_GADGET
public:
    DECLARE_PROPERTY(QString, matched, );
};

class GameRepository : public QObject
{
    Q_OBJECT
public:
    explicit GameRepository(QObject *parent = nullptr);
    ~GameRepository() override;

    bool open(const QString& dbPath, QString *errorString = nullptr);

    std::optional<MemberInfo> createMember(
            const QString &fistName, const QString &lastName,
            const QString &gender, int level);

    [[nodiscard]] QVector<MemberSearchResult> findMember(const QString &needle) const;
    std::optional<MemberInfo> getMember(MemberId) const;
    std::optional<Player> getPlayer(PlayerId) const;
    std::optional<Player> checkIn(MemberId, SessionId, int payment) const;
    bool checkOut(PlayerId);

    [[nodiscard]] std::optional<SessionData> getLastSession() const;
    std::optional<SessionData> createSession(int fee, const QString &announcement, const QVector<CourtConfiguration> &);

    [[nodiscard]] QVector<GameAllocation> getPastAllocations(SessionId) const;
    [[nodiscard]] QVector<GameAllocation> getLastGameAllocation(SessionId) const;

    std::optional<GameId> createGame(SessionId, const QVector<GameAllocation> &);

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
