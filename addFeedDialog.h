#ifndef ADDNEWFEED_H
#define ADDNEWFEED_H

#include "ui_addFeedDialog.h"

#include <QDialog>
#include <QList>

class AddFeedDialog : public QDialog
{
    Q_OBJECT

    public:
        AddFeedDialog(QList<QString> labelList, QWidget *parent = 0);

    private slots:
        void showNewLabel(int);
        void done(int);

    signals:
        void dataReady(QString, QString, QString);

    private:
        Ui::addFeedDialog ui;
};

#endif // ADDNEWFEED_H
