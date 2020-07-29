//
// Created by Fanchao Liu on 29/07/20.
//

#include "MemberLabel.h"

#include <QPainter>
#include <QDebug>


MemberLabel::MemberLabel(QWidget *parent): QLabel(parent) {
}

void MemberLabel::paintEvent(QPaintEvent *event) {
    auto minSize = fontMetrics().boundingRect(text()).size();
    if (minSize.isEmpty()) {
        return;
    }

    qreal scale = 1.0;
    if (size().width() < minSize.width()) {
        scale = static_cast<qreal>(size().width()) / minSize.width();
    }

    if (size().height() < minSize.height()) {
        scale = std::min(scale, static_cast<qreal>(size().height()) / minSize.height());
    }

    QPainter painter(this);
    painter.scale(scale, scale);
    QRectF rect(0.0, 0.0, width() / scale, height() / scale);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextSingleLine, text());
}
