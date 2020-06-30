//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_SESSIONDIALOG_H
#define GAMEMATCHER_SESSIONDIALOG_H

#include <QDialog>

#include "models.h"

class ClubRepository;
class SessionData;

class SessionDialog : public QDialog {
Q_OBJECT
public:
    explicit SessionDialog(ClubRepository *, SessionId, QWidget *parent = nullptr);

    ~SessionDialog() override;

private slots:
    void onSessionDataChanged();
    void onCurrentGameChanged();

    void on_checkInButton_clicked();
    void on_checkOutButton_clicked();
    void on_pauseButton_clicked();
    void on_resumeButton_clicked();
    void on_registerButton_clicked();
    void on_updateButton_clicked();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_SESSIONDIALOG_H
