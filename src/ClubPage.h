//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_CLUBPAGE_H
#define GAMEMATCHER_CLUBPAGE_H


#include <QFrame>
#include <memory>

namespace Ui {
    class ClubPage;
}

class ClubPage : public QFrame {
Q_OBJECT
public:
    explicit ClubPage(const QString &path, QWidget *parent = nullptr);

    ~ClubPage() override;

private slots:
    void onSessionCreated();

private:
    struct Impl;
    std::unique_ptr<Impl> d;

    std::unique_ptr<Ui::ClubPage> ui;
};

#endif //GAMEMATCHER_CLUBPAGE_H
