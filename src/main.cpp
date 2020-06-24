#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QFile>
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

    GameMatcher m({}, {}, {});
    QObject::connect(&m, &GameMatcher::onFinished, [](const auto &result) {
        qDebug() << "Got allocation: " << result.at(0).gameId;
    });
    GameStats stats({}, {}, {});

    return app.exec();
}
