//
// Created by Fanchao Liu on 29/07/20.
//

#ifndef GAMEMATCHER_MEMBERLABEL_H
#define GAMEMATCHER_MEMBERLABEL_H

#include <QLabel>

class MemberLabel : public QLabel {
    Q_OBJECT
public:
    MemberLabel(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};


#endif //GAMEMATCHER_MEMBERLABEL_H
