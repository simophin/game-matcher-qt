//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_CLUBPAGE_H
#define GAMEMATCHER_CLUBPAGE_H


#include <QFrame>
#include "models.h"

class ClubRepository;

class ClubPage : public QFrame {
Q_OBJECT
public:
    static ClubPage *create(const QString &dbPath, QWidget *parent);

    ~ClubPage() override;

signals:
    void clubClosed();

public slots:
    void openSession(SessionId);
    void openLastSession();

private:
    explicit ClubPage(ClubRepository *, QWidget *parent);

    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_CLUBPAGE_H
