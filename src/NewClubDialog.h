//
// Created by Fanchao Liu on 26/06/20.
//

#ifndef GAMEMATCHER_NEWCLUBDIALOG_H
#define GAMEMATCHER_NEWCLUBDIALOG_H

#include <QDialog>
#include <memory>

namespace Ui {
    class NewClubDialog;
}

class NewClubDialog : public QDialog {
Q_OBJECT
public:
    explicit NewClubDialog(QWidget *parent = nullptr);

    ~NewClubDialog() override;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void clubCreated(QString path);

public slots:
    void validateForm();

    void accept() override;

private:
    std::unique_ptr<Ui::NewClubDialog> ui;
};

#endif //GAMEMATCHER_NEWCLUBDIALOG_H
