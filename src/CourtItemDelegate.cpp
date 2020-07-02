//
// Created by Fanchao Liu on 2/07/20.
//

#include "CourtItemDelegate.h"
#include "ClubRepository.h"

#include <QApplication>
#include <QPainter>

struct CourtItemDelegate::Impl {
};

CourtItemDelegate::CourtItemDelegate(QObject *parent) : QAbstractItemDelegate(parent), d(new Impl) {}

void CourtItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto courtPlayers = index.data(Qt::UserRole).value<CourtPlayers>();
    painter->drawRoundedRect(1, 1, painter->device()->width() - 1, painter->device()->height() - 1, 20, 20);

}

QSize CourtItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto courtPlayers = index.data(Qt::UserRole).value<CourtPlayers>();
    return QSize(100, 100);
}

CourtItemDelegate::~CourtItemDelegate() {
    delete d;
}
