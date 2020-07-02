//
// Created by Fanchao Liu on 29/06/20.
//

#ifndef GAMEMATCHER_COURTDISPLAY_H
#define GAMEMATCHER_COURTDISPLAY_H


#include <QWidget>


class CourtPlayers;

class CourtDisplay : public QWidget {
Q_OBJECT
public:
    explicit CourtDisplay(QWidget *parent = nullptr);

    ~CourtDisplay() override;

    void setCourt(const CourtPlayers&);

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_COURTDISPLAY_H
