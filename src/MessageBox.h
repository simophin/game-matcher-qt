//
// Created by Fanchao Liu on 26/08/20.
//

#ifndef GAMEMATCHER_MESSAGEBOX_H
#define GAMEMATCHER_MESSAGEBOX_H

#include <QMessageBox>

static inline void showCritical(QWidget *parent, const QString & title, const QString &body) {
    (new QMessageBox(QMessageBox::Critical, title, body, QMessageBox::Ok, parent))->show();
}

#endif //GAMEMATCHER_MESSAGEBOX_H
