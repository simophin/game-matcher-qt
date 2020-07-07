//
// Created by Fanchao Liu on 8/07/20.
//

#ifndef GAMEMATCHER_TOASTDIALOG_H
#define GAMEMATCHER_TOASTDIALOG_H


#include <QDialog>

class ToastDialog : public QDialog {
Q_OBJECT
public:
    explicit ToastDialog(QWidget *parent = nullptr);

    void showMessage(const QString &, int delayMills);

    ~ToastDialog() override;

    void changeEvent(QEvent *) override;

    signals:
    void showRequested();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_TOASTDIALOG_H
