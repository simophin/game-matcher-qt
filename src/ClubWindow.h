//
// Created by Fanchao Liu on 22/07/20.
//

#ifndef GAMEMATCHER_CLUBWINDOW_H
#define GAMEMATCHER_CLUBWINDOW_H


#include <QMainWindow>

class ClubRepository;

class ClubWindow : public QMainWindow {
Q_OBJECT
public:
    static ClubWindow *create(const QString &dbPath, QWidget *parent);

    ~ClubWindow() override;

    void changeEvent(QEvent *) override;

private:
    ClubWindow(ClubRepository *, QWidget *parent);

    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_CLUBWINDOW_H
