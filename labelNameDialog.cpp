#include "labelNameDialog.h"
#include <QMessageBox>

/*!
\brief Implicitni konstruktor
*/
LabelNameDialog::LabelNameDialog(QList<QString> labelList, QWidget* parent) : QDialog(parent) {

    ui.setupUi(this);

    ui.labelName->hide();
    ui.labelLabelName->hide();

    ui.labelComboBox->addItems(labelList);

    connect(ui.labelComboBox, SIGNAL(activated(int)),
            this, SLOT(showNewLabel(int)));

}

/*!
\brief Zkontroluje zadane informace
*/
void LabelNameDialog::done(int ready) {
    //pokud je stisknuto tlacitko OK
    if (ready == QDialog::Accepted) {
        //zkontroluji povinne udaje
        if (ui.labelName->text().isEmpty() && ui.labelComboBox->currentIndex() == 1) {
            QMessageBox::information(0, trUtf8("Chyba"), trUtf8("Vyplňte název štítku!"));
            return;
        }
        else {
            //odeslu udaje
            if (ui.labelName->isHidden()) {
                if (ui.labelComboBox->currentIndex() == 1) {
                    emit signalLabelName(ui.labelName->text());
                }
                else if (ui.labelComboBox->currentIndex() == 0) {
                    emit signalLabelName("");
                }
                else {
                    emit signalLabelName(ui.labelComboBox->currentText());
                }
            }
            else {
               emit signalLabelName(ui.labelName->text());
            }
        }
        QDialog::done(ready);
    }
    else {
        QDialog::done(ready);
    }
}

/*!
\brief Pokud je zvolena moznost pridat novy stitek zobrazi pole pro jeho zadani
*/
void LabelNameDialog::showNewLabel(int index) {
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
