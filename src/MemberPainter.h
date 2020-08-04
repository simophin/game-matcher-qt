//
// Created by Fanchao Liu on 25/07/20.
//

#ifndef GAMEMATCHER_MEMBERPAINTER_H
#define GAMEMATCHER_MEMBERPAINTER_H

#include <QColor>

class QPainter;

struct BaseMember;

class MemberPainter {
public:
    static QColor colorForMember(const BaseMember &);
};


#endif //GAMEMATCHER_MEMBERPAINTER_H
