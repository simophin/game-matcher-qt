#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include <QObject>

class QFile;

class GameRepository : public QObject
{
    Q_OBJECT
public:
    GameRepository(QObject *parent = nullptr);
    ~GameRepository();

    bool open(const QString& dbPath, QString *errorString = nullptr);


private:
    struct Impl;
    Impl *d;
};

#endif // GAMEREPOSITORY_H
