//
// Created by Fanchao Liu on 8/07/20.
//

#ifndef GAMEMATCHER_TOASTEVENT_H
#define GAMEMATCHER_TOASTEVENT_H

#include <QEvent>
#include <QString>
#include <QString>

class ToastEvent : public QEvent {
public:
    static void show(const QString &msg, int ms = 2000);

    const QString &msg() const { return msg_; }
    long delayMills() const { return delayMills_; }

    static Type eventType();

private:
    ToastEvent(const QString &msg, long delayMills);

private:
    QString msg_;
    long delayMills_;
};


#endif //GAMEMATCHER_TOASTEVENT_H
