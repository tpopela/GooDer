#include "addFeedDialog.h"
#include <QMessageBox>

/*!
\brief Implicitni konstruktor
*/
AddFeedDialog::AddFeedDialog(QList<QString> labelList, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    ui.labelName->hide();
    ui.labelLabelName->hide();

    ui.labelComboBox->addItems(labelList);

    connect(ui.labelComboBox, SIGNAL(activated(int)),
            this, SLOT(showNewLabel(int)));
}

/*!
\brief Zkontroluje zadane udaje
*/
void AddFeedDialog::done(int ready) {
    //pokud je zmacknuto tlacitko OK
    if (QDialog::Accepted == ready) {
        //zkontroluji povinne udaje
        if (ui.feedAddress->text() == "") {
            QMessageBox::information(0, trUtf8("Error"), trUtf8("Please fill feed address!"));
            return;
        }
        else {
            //odeslu udaje
            if (ui.labelName->isHidden()) {
                if (ui.labelComboBox->currentIndex() == 0) {
                    emit dataReady(ui.feedAddress->text(), ui.feedName->text(), "");
                }
                else {
                    emit dataReady(ui.feedAddress->text(), ui.feedName->text(), ui.labelComboBox->currentText());
                }
            }
            else {
                emit dataReady(ui.feedAddress->text(), ui.feedName->text(), ui.labelName->text());
            }
            QDialog::done(ready);
        }
    }
    else {
        QDialog::done(ready);
    }
}

/*!
\brief Pokud je zvolena volba pridat novy stitek zobrazi pole pro jeho zadani
*/
void AddFeedDialog::showNewLabel(int index) {
    if (index == 1) {
        ui.labelLabelName->show();
        ui.labelName->show();
    }
    else {
        ui.labelLabelName->hide();
        ui.labelName->hide();
        ui.labelName->clear();
    }
}
