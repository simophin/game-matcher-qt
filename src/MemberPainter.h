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
    static void paintBackground(QPainter &, const Member &);
    static void paintForeground(QPainter &, const Member &);

    static QColor colorForMember(const Member &);
};


#endif //GAMEMATCHER_MEMBERPAINTER_H
