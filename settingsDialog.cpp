#include "settingsDialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>
#include <QSettings>

/*!
\brief Implicitni konstruktor
*/
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);

    _settings = new QSettings("GooDer", "GooDer");
    _crypt = new SimpleCrypt(Q_UINT64_C(0x0c2ac1a5cea3fe23));

    ui.lineUsername->setText(_settings->value("username").toString());
    if (!_settings->value("password").toString().isNull())
        ui.linePassword->setText(_crypt->decryptToString(_settings->value("password").toString()));
    ui.timeCheckFeeds->setValue(_settings->value("checkFeedTime", 300000).toInt()/60000);

    if (_settings->value("toolbar", true).toBool())
        ui.showToolbar->setChecked(true);
    if (_settings->value("autoHideFeedPanel", true).toBool())
        ui.autoHideFeedPanel->setChecked(true);
    if (_settings->value("menuVisibility", true).toBool())
        ui.showMenu->setChecked(true);
    if (_settings->value("flash", true).toBool())
        ui.flashOnOff->setChecked(true);
    if (_settings->value("showSummary", true).toBool())
        ui.showSummary->setChecked(true);

    ui.shortNextEntry->setText(_settings->value("shortcutNextEntry", "Meta+.").toString());
    ui.shortPrevEntry->setText(_settings->value("shortcutPrevEntry", "Meta+,").toString());
    ui.shortNextFeed->setText(_settings->value("shortcutNextFeed", "Meta+'").toString());
    ui.shortPrevFeed->setText(_settings->value("shortcutPrevFeed", "Meta+;").toString());
    ui.shortNextLabel->setText(_settings->value("shortcutNextLabel", "Meta+]").toString());
    ui.shortPrevLabel->setText(_settings->value("shortcutPrevLabel", "Meta+[").toString());
    ui.shortShowHideFeedList->setText(_settings->value("shortcutToggleFeedPanel", "Meta+\\").toString());

    //install filters for keyboard shortcuts
    ui.shortNextEntry->installEventFilter(this);
    ui.shortNextFeed->installEventFilter(this);
    ui.shortNextLabel->installEventFilter(this);
    ui.shortPrevEntry->installEventFilter(this);
    ui.shortPrevFeed->installEventFilter(this);
    ui.shortPrevLabel->installEventFilter(this);
    ui.shortShowHideFeedList->installEventFilter(this);
}

/*!
\brief Zkontroluje zadane informace.
*/
void SettingsDialog::done(int ready) {

    //pokud je zmacknuto tlacitko OK
    if (ready == QDialog::Accepted) {
        //zkontroluji zadane udaje
        if (ui.lineUsername->text().isEmpty() || ui.linePassword->text().isEmpty() || ui.timeCheckFeeds->value() < 1) {
            QMessageBox::information(0, trUtf8("Error"), trUtf8("Please fill all required informations!"));
            return;
        }
        else if (ui.shortNextEntry->text().isEmpty() || ui.shortNextFeed->text().isEmpty() || ui.shortNextLabel->text().isEmpty()
            || ui.shortPrevEntry->text().isEmpty() || ui.shortPrevFeed->text().isEmpty() || ui.shortPrevLabel->text().isEmpty() || ui.shortShowHideFeedList->text().isEmpty()) {
            QMessageBox::information(0, trUtf8("Error"), trUtf8("Please define all keybord shortcuts!"));
            return;
        }
        //vratim vysledky
        else {
            _settings->setValue("username", ui.lineUsername->text());
            _settings->setValue("password", _crypt->encryptToString(ui.linePassword->text()));
            _settings->setValue("checkFeedTime", ui.timeCheckFeeds->text().toInt()*60000);
            _settings->setValue("toolbar", ui.showToolbar->isChecked());
            _settings->setValue("autoHideFeedPanel", ui.autoHideFeedPanel->isChecked());
            _settings->setValue("menuVisibility", ui.showMenu->isChecked());
            _settings->setValue("flash", ui.flashOnOff->isChecked());
            _settings->setValue("showSummary", ui.showSummary->isChecked());
            _settings->setValue("shortcutNextEntry", ui.shortNextEntry->text().remove(" "));
            _settings->setValue("shortcutPrevEntry", ui.shortPrevEntry->text().remove(" "));
            _settings->setValue("shortcutNextFeed", ui.shortNextFeed->text().remove(" "));
            _settings->setValue("shortcutPrevFeed", ui.shortPrevFeed->text().remove(" "));
            _settings->setValue("shortcutNextLabel", ui.shortNextLabel->text().remove(" "));
            _settings->setValue("shortcutPrevLabel", ui.shortPrevLabel->text().remove(" "));
            _settings->setValue("shortcutToggleFeedPanel", ui.shortShowHideFeedList->text().remove(" "));

            emit settingsData();
            QDialog::done(ready);
        }
    }
    else {
        if (ui.lineUsername->text().isEmpty() || ui.linePassword->text().isEmpty() || ui.timeCheckFeeds->value() < 1)
            emit signalNoUserInfo();
        QDialog::done(ready);
    }
}

/*!
\brief Funkce pro odchyceni klavesovych zkratek - pres nainstalovany filtr
*/
bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.shortNextEntry || obj == ui.shortPrevEntry ||
        obj == ui.shortNextLabel || obj == ui.shortPrevLabel ||
        obj == ui.shortNextFeed || obj == ui.shortPrevFeed ||
        obj == ui.shortShowHideFeedList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Control) {
                ((QLineEdit*) obj)->setText("Ctrl + ");
            }
            if (keyEvent->key() == Qt::Key_Meta) {
                ((QLineEdit*) obj)->setText("Meta + ");
            }
             if (keyEvent->key() == Qt::Key_A) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("A"));
             }
             if (keyEvent->key() == Qt::Key_B) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("B"));
             }
             if (keyEvent->key() == Qt::Key_C) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("C"));
             }
             if (keyEvent->key() == Qt::Key_D) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("D"));
             }
             if (keyEvent->key() == Qt::Key_E) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("E"));
             }
             if (keyEvent->key() == Qt::Key_F) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("F"));
             }
             if (keyEvent->key() == Qt::Key_G) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("G"));
             }
             if (keyEvent->key() == Qt::Key_H) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("H"));
             }
             if (keyEvent->key() == Qt::Key_I) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("I"));
             }
             if (keyEvent->key() == Qt::Key_J) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("J"));
             }
             if (keyEvent->key() == Qt::Key_K) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("K"));
             }
             if (keyEvent->key() == Qt::Key_L) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("L"));
             }
             if (keyEvent->key() == Qt::Key_M) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("M"));
             }
             if (keyEvent->key() == Qt::Key_N) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("N"));
             }
             if (keyEvent->key() == Qt::Key_O) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("O"));
             }
             if (keyEvent->key() == Qt::Key_P) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("P"));
             }
             if (keyEvent->key() == Qt::Key_Q) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Q"));
             }
             if (keyEvent->key() == Qt::Key_R) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("R"));
             }
             if (keyEvent->key() == Qt::Key_S) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("S"));
             }
             if (keyEvent->key() == Qt::Key_T) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("T"));
             }
             if (keyEvent->key() == Qt::Key_U) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("U"));
             }
             if (keyEvent->key() == Qt::Key_V) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("V"));
             }
             if (keyEvent->key() == Qt::Key_W) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("W"));
             }
             if (keyEvent->key() == Qt::Key_X) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("X"));
             }
             if (keyEvent->key() == Qt::Key_Y) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Y"));
             }
             if (keyEvent->key() == Qt::Key_Z) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Z"));
             }


             if (keyEvent->key() == Qt::Key_1) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("1"));
             }
             if (keyEvent->key() == Qt::Key_2) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("2"));
             }
             if (keyEvent->key() == Qt::Key_3) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("3"));
             }
             if (keyEvent->key() == Qt::Key_4) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("4"));
             }
             if (keyEvent->key() == Qt::Key_5) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("5"));
             }
             if (keyEvent->key() == Qt::Key_6) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("6"));
             }
             if (keyEvent->key() == Qt::Key_7) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("7"));
             }
             if (keyEvent->key() == Qt::Key_8) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("8"));
             }
             if (keyEvent->key() == Qt::Key_9) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("9"));
             }
             if (keyEvent->key() == Qt::Key_0) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("0"));
             }

             if (keyEvent->key() == Qt::Key_Colon) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append(":"));
             }
             if (keyEvent->key() == Qt::Key_Slash) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("/"));
             }
             if (keyEvent->key() == Qt::Key_Backslash) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("\\"));
             }
             if (keyEvent->key() == Qt::Key_Comma) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append(","));
             }
             if (keyEvent->key() == Qt::Key_Insert) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Insert"));
             }
             if (keyEvent->key() == Qt::Key_Delete) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Delete"));
             }
             if (keyEvent->key() == Qt::Key_Left) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Left"));
             }
             if (keyEvent->key() == Qt::Key_Right) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Right"));
             }
             if (keyEvent->key() == Qt::Key_Up) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Up"));
             }
             if (keyEvent->key() == Qt::Key_Down) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("Down"));
             }
             if (keyEvent->key() == Qt::Key_Exclam) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("!"));
             }
             if (keyEvent->key() == Qt::Key_QuoteDbl) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("\""));
             }
             if (keyEvent->key() == Qt::Key_Dollar) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("$"));
             }
             if (keyEvent->key() == Qt::Key_Percent) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("%"));
             }
             if (keyEvent->key() == Qt::Key_Ampersand) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("&"));
             }
             if (keyEvent->key() == Qt::Key_Apostrophe) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("'"));
             }
             if (keyEvent->key() == Qt::Key_NumberSign) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("#"));
             }
             if (keyEvent->key() == Qt::Key_ParenLeft) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("("));
             }
             if (keyEvent->key() == Qt::Key_ParenRight) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append(")"));
             }
             if (keyEvent->key() == Qt::Key_Asterisk) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("*"));
             }
             if (keyEvent->key() == Qt::Key_Plus) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("+"));
             }
             if (keyEvent->key() == Qt::Key_Minus) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("-"));
             }
             if (keyEvent->key() == Qt::Key_Period) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("."));
             }
             if (keyEvent->key() == Qt::Key_Semicolon) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append(";"));
             }
             if (keyEvent->key() == Qt::Key_Less) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("<"));
             }
             if (keyEvent->key() == Qt::Key_Equal) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("="));
             }
             if (keyEvent->key() == Qt::Key_Greater) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append(">"));
             }
             if (keyEvent->key() == Qt::Key_Question) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("?"));
             }
             if (keyEvent->key() == Qt::Key_At) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("@"));
             }
             if (keyEvent->key() == Qt::Key_BracketLeft) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("["));
             }
             if (keyEvent->key() == Qt::Key_BracketRight) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("]"));
             }
             if (keyEvent->key() == Qt::Key_Underscore) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("_"));
             }
             if (keyEvent->key() == Qt::Key_AsciiCircum) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("^"));
             }
             if (keyEvent->key() == Qt::Key_AsciiTilde) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("~"));
             }
             if (keyEvent->key() == Qt::Key_BraceLeft) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("{"));
             }
             if (keyEvent->key() == Qt::Key_BraceRight) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("}"));
             }
             if (keyEvent->key() == Qt::Key_Bar) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("|"));
             }
             if (keyEvent->key() == Qt::Key_QuoteLeft) {
                ((QLineEdit*) obj)->setText(((QLineEdit*) obj)->text().append("`"));
             }
         return true;
     } else {
         return false;
     }
 } else {
     return QDialog::eventFilter(obj, event);
 }
}
