//
// Created by Fanchao Liu on 1/07/20.
//

#ifndef GAMEMATCHER_NEWGAMEDIALOG_H
#define GAMEMATCHER_NEWGAMEDIALOG_H


#include <QDialog>

#include "models.h"

class QListWidgetItem;
class ClubRepository;

class NewGameDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewGameDialog(SessionId, ClubRepository *, QWidget *parent = nullptr);

    ~NewGameDialog() override;

    void changeEvent(QEvent *) override;

    void accept() override;

    signals:
    void newGameMade();

private slots:
    void refresh();

    void on_playerList_customContextMenuRequested(const QPoint &);
    void on_playerList_itemDoubleClicked(QListWidgetItem *);

    void validateForm();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_NEWGAMEDIALOG_H