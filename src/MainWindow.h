//
// Created by Fanchao Liu on 26/06/20.
//

#ifndef GAMEMATCHER_MAINWINDOW_H
#define GAMEMATCHER_MAINWINDOW_H

#include <QMainWindow>


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Impl;
    Impl *d;
};

#endif //GAMEMATCHER_MAINWINDOW_H
