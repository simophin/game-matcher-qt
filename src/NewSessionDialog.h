//
// Created by Fanchao Liu on 27/06/20.
//

#ifndef GAMEMATCHER_NEWSESSIONDIALOG_H
#define GAMEMATCHER_NEWSESSIONDIALOG_H


#include <QDialog>
#include <memory>


class ClubRepository;

class NewSessionDialog : public QDialog {
Q_OBJECT
public:
    explicit NewSessionDialog(ClubRepository *, QWidget *parent = nullptr);

    ~NewSessionDialog() override;

    void accept() override;

signals:
    void sessionCreated();

private slots:
    void validateForm();
    void on_buttonBox_accepted();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

#endif //GAMEMATCHER_NEWSESSIONDIALOG_H
