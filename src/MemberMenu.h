//
// Created by Fanchao Liu on 26/07/20.
//

#ifndef GAMEMATCHER_MEMBERMENU_H
#define GAMEMATCHER_MEMBERMENU_H

#include "models.h"

class QPoint;
class QRect;
class ClubRepository;

class MemberMenu {
public:
    static void showAt(
            QWidget *parent,
            ClubRepository *,
            std::optional<SessionId>,
            const Member &m,
            const QPoint &globalPos,
            QRect *itemRect = nullptr);
};


#endif //GAMEMATCHER_MEMBERMENU_H
