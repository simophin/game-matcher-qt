//
// Created by Fanchao Liu on 30/06/20.
//

#ifndef GAMEMATCHER_MEMBERSELECTDIALOG_H
#define GAMEMATCHER_MEMBERSELECTDIALOG_H

#include <QDialog>
#include <QVector>

#include <variant>
#include "models.h"
#include "MemberFilter.h"

class QListWidgetItem;

class ClubRepository;

class MemberSelectDialog : public QDialog {
Q_OBJECT
public:
    explicit MemberSelectDialog(MemberSearchFilter,
            bool showRegister,
            bool closeWhenSelected,
            ClubRepository *, QWidget *parent = nullptr);

    ~MemberSelectDialog() override;

    void setAcceptButtonText(const QString &);

    void accept() override;

protected:
    void changeEvent(QEvent *event) override;

signals:

    void memberSelected(MemberId);

private slots:
    void reload();
    void on_memberList_itemDoubleClicked(QListWidgetItem *item);
    void on_registerButton_clicked();
    void validateForm();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_MEMBERSELECTDIALOG_H
