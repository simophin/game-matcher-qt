//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_SESSIONPAGE_H
#define GAMEMATCHER_SESSIONPAGE_H

#include <QWidget>

#include "models.h"

class ClubRepository;
class SessionData;

class SessionPage : public QWidget {
Q_OBJECT
public:
    static SessionPage *create(SessionId, ClubRepository *, QWidget *parent);
    SessionId sessionId() const;

    ~SessionPage() override;

    signals:
    void closeSessionRequested();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onSessionDataChanged();
    void onCurrentGameChanged();
    void updateElapseTime();

    void showMemberMenuAt(const Member &, const QPoint &);

    void on_wardenOptionButton_clicked();

private:
    struct Impl;
    Impl *d;

    SessionPage(Impl*, QWidget *parent);
};


#endif //GAMEMATCHER_SESSIONPAGE_H
