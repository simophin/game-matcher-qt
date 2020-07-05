//
// Created by Fanchao Liu on 5/07/20.
//

#ifndef GAMEMATCHER_PLAYERTABLEPAGE_H
#define GAMEMATCHER_PLAYERTABLEPAGE_H


#include <QWidget>
#include "models.h"

class ClubRepository;

class PlayerTablePage : public QWidget {
Q_OBJECT
public:
    explicit PlayerTablePage(SessionId, ClubRepository *, QWidget *parent = nullptr);

    ~PlayerTablePage() override;

    void changeEvent(QEvent *) override;

public slots:
    void reload();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_PLAYERTABLEPAGE_H