#include "entry.h"

/*!
\brief Vychozi konstuktor
*/
Entry::Entry() {
    title = "";
    link = "";
    published.setDate(QDate(0,0,0));
    id = "";
    author = "";
    summary = "";
    f_read = true;
}

/*!
\brief Konstruktor s naplnenim dat
*/
Entry::Entry(QString p_title, QString p_published, QString p_link, QString p_id, QString p_author, bool p_read) {
    QString format = "yyyy-MM-dd'T'HH:mm:ss'Z'";

    title = p_title;
    published = QDateTime::fromString(p_published, format);
    link = p_link;
    id = p_id;
    f_read = p_read;
    author = p_author;
}

/*!
\brief Konstruktor s naplnenim dat
*/
Entry::Entry(QString p_title, QString p_published, QString p_link, QString p_id, QString p_author, QString p_summary, bool p_read) {
    QString format = "yyyy-MM-dd'T'HH:mm:ss'Z'";

    title = p_title;
    published = QDateTime::fromString(p_published, format);
    link = p_link;
    id = p_id;
    f_read = p_read;
    author = p_author;
    summary = p_summary;
}
