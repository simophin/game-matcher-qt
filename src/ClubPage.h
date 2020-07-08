//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_CLUBPAGE_H
#define GAMEMATCHER_CLUBPAGE_H


#include <QFrame>
#include "models.h"


class ClubPage : public QFrame {
Q_OBJECT
public:
    explicit ClubPage(const QString &path, QWidget *parent = nullptr);

    ~ClubPage() override;

signals:
    void clubClosed();

public slots:
    void openSession(SessionId);
    void openLastSession();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_CLUBPAGE_H
