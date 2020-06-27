//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_CLUBHOMEPAGE_H
#define GAMEMATCHER_CLUBHOMEPAGE_H


#include <QFrame>
#include <memory>

namespace Ui {
    class ClubHomePage;
}

class ClubHomePage : public QFrame {
Q_OBJECT
public:
    explicit ClubHomePage(QWidget *parent = nullptr);

    ~ClubHomePage() override;

private:
    std::unique_ptr<Ui::ClubHomePage> ui;
};

#endif //GAMEMATCHER_CLUBHOMEPAGE_H
