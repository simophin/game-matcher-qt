//
// Created by Fanchao Liu on 28/06/20.
//

#ifndef GAMEMATCHER_EMPTYSESSIONPAGE_H
#define GAMEMATCHER_EMPTYSESSIONPAGE_H

#include <QFrame>

class ClubRepository;

class EmptySessionPage : public QFrame {
Q_OBJECT
public:
    explicit EmptySessionPage(ClubRepository *, QWidget *parent = nullptr);

    ~EmptySessionPage() override;

    signals:
    void newSessionCreated();
    void lastSessionResumed();
    void clubClosed();

private slots:
    void reload();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_EMPTYSESSIONPAGE_H
