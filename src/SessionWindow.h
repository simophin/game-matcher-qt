//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_SESSIONWINDOW_H
#define GAMEMATCHER_SESSIONWINDOW_H

#include <QMainWindow>

#include "models.h"

class ClubRepository;
class SessionData;

class SessionWindow : public QMainWindow {
Q_OBJECT
public:
    explicit SessionWindow(ClubRepository *, SessionId, QWidget *parent = nullptr);

    ~SessionWindow() override;

protected:
    void changeEvent(QEvent *event) override;

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSessionDataChanged();
    void onCurrentGameChanged();
    void updateElapseTime();

    void on_checkInButton_clicked();
    void on_checkOutButton_clicked();
    void on_pauseButton_clicked();
    void on_resumeButton_clicked();
    void on_updateButton_clicked();

    void on_actionStartNewGame_triggered();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_SESSIONWINDOW_H
