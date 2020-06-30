//
// Created by Fanchao Liu on 30/06/20.
//

#ifndef GAMEMATCHER_EDITMEMBERDIALOG_H
#define GAMEMATCHER_EDITMEMBERDIALOG_H


#include <QDialog>
#include "models.h"

class ClubRepository;

class EditMemberDialog : public QDialog {
Q_OBJECT
public:
    explicit EditMemberDialog(ClubRepository *, QWidget *parent = nullptr);

    ~EditMemberDialog() override;

    void setMember(MemberId);

    void accept() override;

    signals:
    void newMemberCreated(MemberId);
    void memberUpdated(MemberId);

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_EDITMEMBERDIALOG_H
