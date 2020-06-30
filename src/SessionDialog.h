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

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_SESSIONDIALOG_H
