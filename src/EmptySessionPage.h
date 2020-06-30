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

private slots:
    void applyInfo();
    void on_startButton_clicked();
    void on_resumeButton_clicked();
    void on_updateButton_clicked();
    void on_statsButton_clicked();
    void on_newMemberButton_clicked();
    void on_createFakeButton_clicked();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_EMPTYSESSIONPAGE_H
