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

    QCoreApplication app(argc, argv);
    GameRepository repo;
    QString error;
    if (!repo.open(QLatin1String(":memory:"), &error)) {
        qDebug() << "Error opening db: " << error;
    }

    QRandomGenerator random(1);
    QVector<Member> members;
    for (auto i = 0; i < 20; i++) {
        Member m;
        m.id = i;
        m.level = random.bounded(10);
        m.fistName = QStringLiteral("Player %1").arg(i + 1);
        m.lastName = "Last";
        m.gender = i % 3 == 0 ? "male" : "female";
        members.push_back(m);
    }

    QVector<Player> players;
    for (const auto &member : members) {
        Player p;
        p.memberId = member.id;
        p.id = member.id;
        p.checkInTime = QDateTime::currentDateTime();
        players.push_back(p);
    }

    QVector<CourtId> courts = { 1, 2, 3, 4 };

    QVector<GameAllocation> pastAllocations;

    auto onGameMatched = [&](const QVector<GameAllocation> &result) {
        qDebug() << "Got allocation: " << result;
        pastAllocations.append(result);

        QObject::connect(new GameMatcher(pastAllocations, members, players, courts, 4, 1), &GameMatcher::onFinished, onGameMatched);
    };

    QObject::connect(new GameMatcher(pastAllocations, members, players, courts, 4, 1), &GameMatcher::onFinished, onGameMatched);

    return app.exec();
}
