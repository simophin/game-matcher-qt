//
// Created by Fanchao Liu on 5/08/20.
//

#ifndef GAMEMATCHER_PLAYERTABLEDIALOG_H
#define GAMEMATCHER_PLAYERTABLEDIALOG_H

#include "models.h"

#include <QDialog>

class ClubRepository;

class PlayerTableDialog : public QDialog {
Q_OBJECT
public:
    PlayerTableDialog(ClubRepository *, SessionId, QWidget *parent = nullptr);

    ~PlayerTableDialog() override;

    void changeEvent(QEvent *) override;

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_PLAYERTABLEDIALOG_H
