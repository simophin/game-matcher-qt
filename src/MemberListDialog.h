//
// Created by Fanchao Liu on 14/08/20.
//

#ifndef GAMEMATCHER_MEMBERLISTDIALOG_H
#define GAMEMATCHER_MEMBERLISTDIALOG_H

#include <QDialog>

class ClubRepository;

class MemberListDialog : public QDialog {
Q_OBJECT
public:
    MemberListDialog(ClubRepository *, QWidget *parent = nullptr);

    ~MemberListDialog() override;

    void changeEvent(QEvent *) override;

private slots:
    void reload();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_MEMBERLISTDIALOG_H
