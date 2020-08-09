//
// Created by Fanchao Liu on 28/07/20.
//

#ifndef GAMEMATCHER_COURTDISPLAYLAYOUT_H
#define GAMEMATCHER_COURTDISPLAYLAYOUT_H

#include <QLayout>

class CourtDisplayLayout : public QLayout {
    Q_OBJECT
public:
    CourtDisplayLayout();
    ~CourtDisplayLayout() override;

    void addItem(QLayoutItem *item) override;

    QSize minimumSize() const override;

    void setGeometry(const QRect &rect) override;

    QLayoutItem *itemAt(int index) const override;

    QLayoutItem *takeAt(int index) override;

    int count() const override;

    QSize sizeHint() const override;

    Qt::Orientations expandingDirections() const override {
        return Qt::Horizontal | Qt::Vertical;
    }

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_COURTDISPLAYLAYOUT_H
