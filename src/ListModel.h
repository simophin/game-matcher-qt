//
// Created by Fanchao Liu on 2/07/20.
//

#ifndef GAMEMATCHER_LISTMODEL_H
#define GAMEMATCHER_LISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

template <typename T>
class ListModel : public QAbstractListModel {
    QVector<T> items_;

public:
    ListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    void setItems(const QVector<T> &items) {
        emit this->layoutAboutToBeChanged();
        items_ = items;
        emit this->layoutChanged();
    }

    const QVector<T> &items() const {
        return this->items_;
    }

    int rowCount(const QModelIndex &parent) const override {
        return items_.size();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        return QVariant::fromValue(items_.at(index.row()));
    }
};


#endif //GAMEMATCHER_LISTMODEL_H
