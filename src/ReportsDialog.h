//
// Created by Fanchao Liu on 12/08/20.
//

#ifndef GAMEMATCHER_REPORTSDIALOG_H
#define GAMEMATCHER_REPORTSDIALOG_H


#include <QDialog>

class ReportsDialog : public QDialog {
Q_OBJECT
public:
    explicit ReportsDialog(QWidget *parent = nullptr);

    ~ReportsDialog() override;

    void changeEvent(QEvent *) override;

private:
    struct Impl;
    Impl *d;
};


#endif //GAMEMATCHER_REPORTSDIALOG_H
