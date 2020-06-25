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

class GameRepository : public QObject
{
    Q_OBJECT
public:
    explicit GameRepository(QObject *parent = nullptr);
    ~GameRepository() override;

    bool open(const QString& dbPath, QString *errorString = nullptr);

    [[nodiscard]] std::optional<SessionData> getLastSession() const;
    std::optional<SessionData> createSession(int fee, const QString &announcement, const QVector<CourtConfiguration> &);

    QVector<GameAllocation> getPastAllocations(SessionId) const;

    GameId createGame(const QVector<GameAllocation> &);

private:
    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
