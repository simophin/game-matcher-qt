//
// Created by Fanchao Liu on 30/06/20.
//

#ifndef GAMEMATCHER_CHECKINDIALOG_H
#define GAMEMATCHER_CHECKINDIALOG_H


#include <QDialog>

#include "models.h"

class ClubRepository;

class CheckInDialog : public QDialog {
Q_OBJECT
public:
    explicit CheckInDialog(MemberId, SessionId, ClubRepository *, QWidget *parent = nullptr);

    ~CheckInDialog() override;

    void accept() override;

    signals:
    void memberCheckedIn(MemberId);

private slots:
    void validateForm();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_CHECKINDIALOG_H
