//
// Created by Fanchao Liu on 25/07/20.
//

#include "MemberPainter.h"
#include "models.h"

#include <QPainter>
#include <QApplication>
#include <QPalette>

#include <vector>

static std::vector<QColor> bgColors = {
        "cornflowerblue",
        "darkblue",
        "darkolivegreen",
        "firebrick",
        "saddlebrown",
        "slategrey",
        "seagreen",
};

void MemberPainter::paintBackground(QPainter &painter, const Member &m) {
    auto bgColor = bgColors[m.level % bgColors.size()];
    painter.fillRect(painter.viewport(), bgColor);
}

void MemberPainter::paintForeground(QPainter &, const Member &) {

}

QColor MemberPainter::colorForMember(const Member &m) {
    return bgColors[m.level % bgColors.size()];
}
