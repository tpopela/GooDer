#ifndef LABELNAMEDIALOG_H
#define LABELNAMEDIALOG_H

#include "ui_labelNameDialog.h"

#include <QObject>

class LabelNameDialog : public QDialog
{
    Q_OBJECT

    public:
        LabelNameDialog(QList<QString> p_labelList, QWidget* parent = 0);

    private slots:
        void done(int);
        void showNewLabel(int);

    signals:
        void signalLabelName(QString);

    private:
        Ui::labelNameDialog ui;
};

#endif // LABELNAMEDIALOG_H
