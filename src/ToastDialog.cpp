//
// Created by Fanchao Liu on 8/07/20.
//

#include "ToastDialog.h"
#include "ui_ToastDialog.h"

#include <QEvent>

#include <QTimer>
#include <QStateMachine>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QPointer>
#include <QStatusBar>

struct ToastDialog::Impl {
    Ui::ToastDialog ui;
    QTimer timer;
};

static QPointer<QMainWindow> toastMainWindow;

ToastDialog::ToastDialog(QWidget *parent)
        : QDialog(parent, Qt::FramelessWindowHint | Qt::CustomizeWindowHint), d(new Impl) {
    d->ui.setupUi(this);

    auto machine = new QStateMachine(this);
    machine->setAnimated(true);
    machine->addDefaultAnimation(new QPropertyAnimation(this, "windowOpacity"));
    auto visibleState = new QState(machine);
    auto invisibleState = new QState(machine);

    visibleState->assignProperty(this, "windowOpacity", 1.0);
    visibleState->assignProperty(this, "visible", true);
    invisibleState->assignProperty(this, "windowOpacity", 0.0);
    invisibleState->assignProperty(this, "visible", false);
    machine->setInitialState(invisibleState);

    invisibleState->addTransition(this, &ToastDialog::showRequested, visibleState);
    visibleState->addTransition(&d->timer, &QTimer::timeout, invisibleState);

    machine->start();
}

ToastDialog::~ToastDialog() {
    delete d;
}

void ToastDialog::changeEvent(QEvent *evt) {
    QDialog::changeEvent(evt);
    if (evt->type() == QEvent::LanguageChange) {
        d->ui.retranslateUi(this);
    }
}

void ToastDialog::showMessage(const QString &msg, int delayMills) {
    d->ui.label->setText(msg);
    adjustSize();
    d->timer.setSingleShot(true);
    d->timer.start(delayMills);
    raise();
    activateWindow();
    emit this->showRequested();
}

void ToastDialog::show(const QString &msg, int delayMills) {
    if (auto window = toastMainWindow.data()) {
        if (auto status = window->statusBar()) {
            status->showMessage(msg, delayMills);
        }
    }
}

void ToastDialog::registerMainWindow(QMainWindow *window) {
    toastMainWindow = window;
}
