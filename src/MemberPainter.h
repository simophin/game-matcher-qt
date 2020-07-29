//
// Created by Fanchao Liu on 25/07/20.
//

#ifndef GAMEMATCHER_MEMBERPAINTER_H
#define GAMEMATCHER_MEMBERPAINTER_H

#include <QColor>

class QPainter;

struct Member;

class MemberPainter {
public:
    static QColor colorForMember(const Member &);
};


#endif //GAMEMATCHER_MEMBERPAINTER_H
