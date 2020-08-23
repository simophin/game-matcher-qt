//
// Created by Fanchao Liu on 24/06/20.
//

#ifndef GAMEMATCHER_GAMESTATS_H
#define GAMEMATCHER_GAMESTATS_H

#include <QSet>
#include <map>
#include <QJsonObject>
#include <QJsonArray>

#include "models.h"
#include "PlayerInfo.h"


class GameStats {
public:
    virtual ~GameStats() = default;

    virtual int numGamesFor(MemberId) const = 0;

    virtual int numGamesOff(MemberId) const = 0;

    virtual int numGames() const = 0;

    virtual int similarityScore(const QVector<MemberId> &) const = 0;
};

class GameStatsImpl : public GameStats {
    std::map<GameId, std::map<CourtId, QSet<MemberId>>> games;
    int numTotalGames = 0;

public:

    explicit GameStatsImpl(const QVector<GameAllocation> &pastAllocation) {
        for (const auto &allocation : pastAllocation) {
            games[allocation.gameId][allocation.courtId].insert(allocation.memberId);
        }

        numTotalGames = games.size();
    }

    explicit GameStatsImpl(const QJsonObject &obj) {
        numTotalGames = obj[QStringLiteral("numTotalGames")].toInt();
        auto gameObj = obj[QStringLiteral("games")].toObject();
        for (auto iter = gameObj.begin(); iter != gameObj.end(); ++iter) {
            bool ok;
            if (auto gameId = iter.key().toLongLong(&ok); ok) {
                auto &game = games[gameId];
                auto courtsObj = iter.value().toObject();
                for (auto courtIter = courtsObj.begin(); courtIter != courtsObj.end(); ++courtIter) {
                    if (auto courtId = courtIter.key().toLongLong(&ok); ok) {
                        auto &courtPlayers = game[courtId];
                        for (auto v : courtIter.value().toArray()) {
                            courtPlayers.insert(v.toInt());
                        }
                    }
                }
            }
        }
    }

    QJsonObject toJson() const {
        QJsonObject gamesObj;
        for (const auto &[id, game]: games) {
            QJsonObject courtObj;
            for (const auto &[courtId, players] : game) {
                QJsonArray memberArray;
                for (auto p : players) {
                    memberArray += p;
                }

                courtObj[QString::number(courtId)] = memberArray;
            }
            gamesObj[QString::number(id)] = courtObj;
        }

        return {
                {QStringLiteral("numTotalGames"), numTotalGames,},
                {QStringLiteral("games"),         gamesObj},
        };
    }

    int numGamesFor(MemberId memberId) const override {
        int rc = 0;
        for (const auto &[id, courts] : games) {
            for (const auto &[courtId, members] : courts) {
                if (members.contains(memberId)) {
                    rc++;
                }
            }
        }
        return rc;
    }

    int numGamesOff(MemberId memberId) const override {
        int i = 0;
        for (auto iter = games.rbegin(); iter != games.rend(); ++iter) {
            for (const auto &court : iter->second) {
                if (court.second.contains(memberId)) {
                    return i;
                }
            }
            i++;
        }

        return i;
    }

    int numGames() const override { return this->numTotalGames; }

    int similarityScore(const QVector<MemberId> &players) const override {
        if (games.empty()) return 0;

        int totalSeats = 0;
        int sum = 0;
        for (const auto &[gameId, game] : games) {
            for (const auto &[courtId, court] : game) {
                int numPlayedHere = 0;
                for (const auto &p : players) {
                    if (court.contains(p)) {
                        numPlayedHere++;
                    }
                }

                if (numPlayedHere >= 2) {
                    totalSeats += std::min(static_cast<int>(court.size()), players.size());
                    sum += numPlayedHere;
                }
            }
        }

        if (totalSeats == 0) return 0;
        return sum * 100 / totalSeats;
    }
};

#endif //GAMEMATCHER_GAMESTATS_H
