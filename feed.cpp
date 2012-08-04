#include "feed.h"

/*!
\brief Vychozi konstruktor
*/
Feed::Feed() {
    id = "";
    unreadCount = 0;
    newestItemTimestampUsec = 0;
    firstItemMsec = 0;
    labelList.empty();
}

/*!
\brief Konstruktor, ve kterem parametry prijimame id zdroje a nazev zdroje
*/
Feed::Feed(QString p_id, QString p_title) {
    id = p_id;
    title = p_title;
    unreadCount = 0;
}

/*!
\brief Konstruktor, ve kterem parametry prijimame id, pocet neprectenych polozek a cas nejnovejsi polozky
*/
Feed::Feed(QString p_id, QString p_title, QList<QString> p_label, QString p_firstItemMsec) {
    id = p_id;
    title = p_title;
    labelList.append(p_label);
    firstItemMsec = p_firstItemMsec.toLong();
    unreadCount = 0;
}
