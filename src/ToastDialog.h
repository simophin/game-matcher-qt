//
// Created by Fanchao Liu on 8/07/20.
//

#ifndef GAMEMATCHER_TOASTDIALOG_H
#define GAMEMATCHER_TOASTDIALOG_H


#include <QDialog>

class QMainWindow;

class ToastDialog : public QDialog {
Q_OBJECT
public:
    explicit ToastDialog(QWidget *parent = nullptr);

    void showMessage(const QString &, int delayMills);

    ~ToastDialog() override;

    void changeEvent(QEvent *) override;

    static void show(const QString&, int delayMills = 20000);
    static void registerMainWindow(QMainWindow *);

    signals:
    void showRequested();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_TOASTDIALOG_H
