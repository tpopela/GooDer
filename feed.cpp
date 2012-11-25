#include "feed.h"

/*!
\brief Vychozi konstruktor
*/
Feed::Feed() {
    _id = "";
    _unreadCount = 0;
    _newestItemTimestampUsec = 0;
    _firstItemMsec = 0;
    _labelList.empty();
}

/*!
\brief Konstruktor, ve kterem parametry prijimame id zdroje a nazev zdroje
*/
Feed::Feed(QString id, QString title) {
    _id = id;
    _title = title;
    _unreadCount = 0;
}

/*!
\brief Konstruktor, ve kterem parametry prijimame id, pocet neprectenych polozek a cas nejnovejsi polozky
*/
Feed::Feed(QString id, QString title, QList<QString> label, QString firstItemMsec) {
    _id = id;
    _title = title;
    _labelList.append(label);
    _firstItemMsec = firstItemMsec.toLong();
    _unreadCount = 0;
}

void Feed::removeLabel(QString label) {
    for (int i = 0; i < _labelList.count(); i++) {
        if (label == _labelList.at(i)) {
            _labelList.removeAt(i);
            break;
        }
    }
}
