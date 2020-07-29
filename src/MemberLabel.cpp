//
// Created by Fanchao Liu on 29/07/20.
//

#include "MemberLabel.h"

#include <QPainter>
#include <QIcon>

const qreal iconSize = 30.0;
const qreal iconSpacing = 8.0;

MemberLabel::MemberLabel(QWidget *parent): QLabel(parent) {
}

void MemberLabel::paintEvent(QPaintEvent *event) {
    auto minSize = fontMetrics().boundingRect(text()).size();
    if (minSize.isEmpty()) {
        return;
    }

    int actualWidth = size().width() - (paid_ ? (iconSize + iconSpacing) * 2 : 0);
    int actualHeight = size().height();
    QPainter painter(this);


    qreal scale = 1.0;
    if (actualWidth < minSize.width()) {
        scale = static_cast<qreal>(actualWidth) / minSize.width();
    }

    if (actualHeight < minSize.height()) {
        scale = std::min(scale, static_cast<qreal>(actualHeight) / minSize.height());
    }


    painter.scale(scale, scale);
    QRectF rect(0.0, 0.0, width() / scale, height() / scale);
    QRectF bounding;
    painter.drawText(rect, Qt::AlignCenter | Qt::TextSingleLine, text(), &bounding);

    if (paid_) {
        paidIcon().paint(&painter, bounding.left() - iconSize - iconSpacing,
                         bounding.top() + (bounding.height() - iconSize) / 2.0,
                         iconSize, iconSize);
    }
}

const QIcon &MemberLabel::paidIcon() {
    static QIcon icon(QStringLiteral(":/icons/Finance/money-dollar-box-line.svg"));
    return icon;
}

void MemberLabel::setPaid(bool paid) {
    if (paid_ != paid) {
        paid_ = paid;
        update();
    }
}
