#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QFile>
#include <QRandomGenerator>
#include "gamerepository.h"
#include "gamematcher.h"
#include "gamestats.h"
#include "models.h"

int main(int argc, char *argv[])
{
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

//    QGuiApplication app(argc, argv);

//    QQmlApplicationEngine engine;
//    const QUrl url(QStringLiteral("qrc:/main.qml"));
//    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
//                     &app, [url](QObject *obj, const QUrl &objUrl) {
//        if (!obj && url == objUrl)
//            QCoreApplication::exit(-1);
//    }, Qt::QueuedConnection);
//    engine.load(url);

    qRegisterMetaType<GameId>("GameId");
    qRegisterMetaType<CourtId>("CourtId");
    qRegisterMetaType<SessionId>("SessionId");
    qRegisterMetaType<PlayerId>("PlayerId");
    qRegisterMetaType<MemberId>("MemberId");

    QCoreApplication app(argc, argv);
    GameRepository repo;
    QString error;
    if (!repo.open(QLatin1String("test.sqlitedb"), &error)) {
        qDebug() << "Error opening db: " << error;
        return -2;
    }

    QRandomGenerator random(1);
    QVector<Member> members;
    for (auto i = 0; i < 20; i++) {
        auto m = repo.createMember(
                QStringLiteral("Player %1").arg(i + 1),
                QStringLiteral("Last"),
                i % 3 == 0 ? QStringLiteral("male") : QStringLiteral("female"),
                random.bounded(10)
                );

        if (!m) {
            qCritical() << "Unable to create member";
            return -1;
        }

        members.push_back(*m);
    }


    auto lastSession = repo.createSession(500, QLatin1String("Hello, world"), {
        CourtConfiguration { QLatin1String("Court1"), 1 },
        CourtConfiguration { QLatin1String("Court2"), 1 },
    });
    if (!lastSession) {
        qCritical() << "Unable to create session";
        return -1;
    }

    lastSession = repo.getLastSession();
    if (!lastSession) {
        qDebug() << "Can't find last session";
        return -1;
    }

    QVector<Player> players;
    for (const auto &member : members) {
        auto p = repo.checkIn(member.id, lastSession->session.id, 500);
        if (!p) {
            qCritical() << "Unable to check in memberId" << member.id;
            return -1;
        }

        players.push_back(*p);
    }

    QVector<CourtId> courts;
    for (const auto &court : lastSession->courts) {
        courts.append(court.id);
    }

    QVector<GameAllocation> pastAllocations;

    GameMatcher matcher;
    auto matchResult = matcher.match(pastAllocations, members, players, courts, 4, 1);
    auto gameId = repo.createGame(lastSession->session.id, matchResult);
    if (!gameId) {
        qCritical() << "Unable to create game";
        return -2;
    }

    pastAllocations.append(repo.getLastGameAllocation(lastSession->session.id));
    gameId = repo.createGame(lastSession->session.id, matcher.match(pastAllocations, members, players, courts, 4, 1));
    if (!gameId) {
        qCritical() << "Unable to create game2";
        return -3;
    }

    return app.exec();
}
