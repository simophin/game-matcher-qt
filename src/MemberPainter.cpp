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
        "darkred",
        "darkorange",
        "darkgoldenrod",
        "darkgreen",
        "darkblue",
        "saddlebrown",
        "slategrey",
};

QColor MemberPainter::colorForMember(const Member &m) {
    return bgColors[(m.level - 1) % bgColors.size()];
}
