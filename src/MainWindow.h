//
// Created by Fanchao Liu on 23/07/20.
//

#ifndef GAMEMATCHER_MAINWINDOW_H
#define GAMEMATCHER_MAINWINDOW_H


#include <QMainWindow>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void changeEvent(QEvent *) override;

private slots:
    bool openClub(const QString &);
    void openWelcomePage();

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_MAINWINDOW_H
