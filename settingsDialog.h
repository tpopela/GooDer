#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "ui_settingsDialog.h"
#include "simplecrypt.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        SettingsDialog(QWidget *parent);

    private slots:
        void done(int);

    protected:
        bool eventFilter(QObject*, QEvent*);

    signals:
        void settingsData();
        void signalNoUserInfo();

    private:
        Ui::settingsDialog ui;
        QSettings* _settings;
        SimpleCrypt* _crypt;
};

#endif // SETTINGSDIALOG_H
