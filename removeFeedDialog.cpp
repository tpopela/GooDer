#include "removeFeedDialog.h"

/*!
\brief Implicitni konstruktor
*/
RemoveFeedDialog::RemoveFeedDialog(QList<Feed*> feedList, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    for (int i = 0; i < feedList.count(); i++)
        ui.feedListWidget->addItem(feedList.at(i)->getTitle());

    connect(ui.buttonBox, SIGNAL(accepted()),
            this, SLOT(getFeedId()));

    connect(ui.buttonBox, SIGNAL(rejected()),
            this, SLOT(close()));
}

/*!
\brief Zjisti ID zvoleneho zdroje pro odstraneni
*/
void RemoveFeedDialog::getFeedId() {
    emit removeFeedData(ui.feedListWidget->currentIndex().row());
    this->accept();
}
