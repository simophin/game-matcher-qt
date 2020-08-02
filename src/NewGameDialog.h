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
    static NewGameDialog *create(SessionId, ClubRepository *, QWidget *parent);

    ~NewGameDialog() override;

    void changeEvent(QEvent *) override;

    void accept() override;

    signals:
    void newGameMade();

private slots:
    void refresh();
    void validateForm();

private:
    struct Impl;
    Impl *d;

    explicit NewGameDialog(Impl *, QWidget *parent);
};


#endif //GAMEMATCHER_NEWGAMEDIALOG_H
