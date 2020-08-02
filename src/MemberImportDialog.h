//
// Created by Fanchao Liu on 2/08/20.
//

#ifndef GAMEMATCHER_MEMBERIMPORTDIALOG_H
#define GAMEMATCHER_MEMBERIMPORTDIALOG_H


#include <QDialog>

class ClubRepository;
class MemberImportDialog : public QDialog {
Q_OBJECT
public:
    explicit MemberImportDialog(ClubRepository *, QWidget *parent = nullptr);

    ~MemberImportDialog() override;

    void changeEvent(QEvent *) override;

    void accept() override;

private slots:
    void reload();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_MEMBERIMPORTDIALOG_H
