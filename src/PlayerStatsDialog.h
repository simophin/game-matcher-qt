//
// Created by Fanchao Liu on 3/08/20.
//

#ifndef GAMEMATCHER_PLAYERSTATSDIALOG_H
#define GAMEMATCHER_PLAYERSTATSDIALOG_H

#include <QDialog>

#include "models.h"

class ClubRepository;

class PlayerStatsDialog : public QDialog {
Q_OBJECT
public:
    PlayerStatsDialog(const BaseMember &, SessionId, ClubRepository *, QWidget *parent = nullptr);

    ~PlayerStatsDialog() override;

    void changeEvent(QEvent *) override;

private slots:
    void reload();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_PLAYERSTATSDIALOG_H
