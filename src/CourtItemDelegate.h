//
// Created by Fanchao Liu on 2/07/20.
//

#ifndef GAMEMATCHER_COURTITEMDELEGATE_H
#define GAMEMATCHER_COURTITEMDELEGATE_H

#include <QAbstractItemDelegate>


class CourtItemDelegate : public QAbstractItemDelegate {
public:
    CourtItemDelegate(QObject *parent = nullptr);

    ~CourtItemDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_COURTITEMDELEGATE_H
