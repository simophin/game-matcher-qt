//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_SESSIONPAGE_H
#define GAMEMATCHER_SESSIONPAGE_H

#include <QFrame>
#include <memory>

class ClubRepository;
class SessionData;

class SessionPage : public QFrame {
Q_OBJECT
public:
    explicit SessionPage(ClubRepository *, const SessionData &, QWidget *parent = nullptr);

    ~SessionPage() override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};


#endif //GAMEMATCHER_SESSIONPAGE_H
