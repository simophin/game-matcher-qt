//
// Created by Fanchao Liu on 26/06/20.
//

#ifndef GAMEMATCHER_MAINWINDOW_H
#define GAMEMATCHER_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onClubOpened(QString path);

private:
    std::unique_ptr<Ui::MainWindow> ui;
};

#endif //GAMEMATCHER_MAINWINDOW_H
