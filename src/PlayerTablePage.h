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
    explicit PlayerTablePage(QWidget *parent = nullptr);

    void load(SessionId, ClubRepository *);

    ~PlayerTablePage() override;

    void changeEvent(QEvent *) override;

public slots:
    void reload();

private slots:
    void on_table_customContextMenuRequested(const QPoint &pt);

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_PLAYERTABLEPAGE_H
