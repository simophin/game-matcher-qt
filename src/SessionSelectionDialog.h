//
// Created by Fanchao Liu on 12/08/20.
//

#ifndef GAMEMATCHER_SESSIONSELECTIONDIALOG_H
#define GAMEMATCHER_SESSIONSELECTIONDIALOG_H

#include <QDialog>

#include "NumericRange.h"
#include "models.h"

class ClubRepository;

class SessionSelectionDialog : public QDialog {
Q_OBJECT
public:
    SessionSelectionDialog(ClubRepository *, SizeRange, QWidget *parent = nullptr);

    ~SessionSelectionDialog() override;

    void changeEvent(QEvent *) override;

    void accept() override;

signals:

    void onSessionSelected(QSet<SessionId>);

private slots:
    void reload();
    void validateForm();

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_SESSIONSELECTIONDIALOG_H
