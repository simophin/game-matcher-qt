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

    static const QIcon &paidIcon();

    void setPaid(bool paid);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool paid_ = false;
};


#endif //GAMEMATCHER_MEMBERLABEL_H
