#ifndef REMOVEFEEDDIALOG_H
#define REMOVEFEEDDIALOG_H

#include <QDialog>
#include "ui_removeFeedDialog.h"
#include "feed.h"

class RemoveFeedDialog : public QDialog
{
    Q_OBJECT

    public:
        RemoveFeedDialog(QList<Feed*>, QWidget *parent = 0);

    signals:
        void removeFeedData(int);

    private slots:
        void getFeedId();

    private:
        Ui::removeFeedDialog ui;
};

#endif // REMOVEFEEDDIALOG_H
