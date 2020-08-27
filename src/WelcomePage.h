//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_WELCOMEPAGE_H
#define GAMEMATCHER_WELCOMEPAGE_H

#include <QFrame>
#include <memory>

namespace Ui {
    class WelcomePage;
}

class WelcomePage : public QFrame {
Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);

    ~WelcomePage() override;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void clubOpened(QString path);

private:
    std::unique_ptr<Ui::WelcomePage> ui;
};

#endif //GAMEMATCHER_WELCOMEPAGE_H
