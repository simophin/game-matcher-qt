//
// Created by Fanchao Liu on 8/07/20.
//

#include "ToastEvent.h"

#include <QCoreApplication>

void ToastEvent::show(const QString &msg, int ms) {
    auto event = new ToastEvent(msg, ms);
    QCoreApplication::postEvent(QCoreApplication::instance(), event);
}

ToastEvent::ToastEvent(const QString &msg, long delayMills)
        : QEvent(eventType()), msg_(msg),
          delayMills_(delayMills) {}

QEvent::Type ToastEvent::eventType() {
    static auto type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

