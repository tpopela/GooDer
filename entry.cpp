#include "entry.h"

/*!
\brief Vychozi konstuktor
*/
Entry::Entry() {
    _title = "";
    _link = "";
    _published.setDate(QDate(0,0,0));
    _id = "";
    _author = "";
    _summary = "";
    _alreadyRead = true;
}

/*!
\brief Konstruktor s naplnenim dat
*/
Entry::Entry(QString title, QString published, QString link, QString id,
    QString author, bool alreadyRead) {

    QString format = "yyyy-MM-dd'T'HH:mm:ss'Z'";

    _title = title;
    _published = QDateTime::fromString(published, format);
    _link = link;
    _id = id;
    _alreadyRead = alreadyRead;
    _author = author;
}

/*!
\brief Konstruktor s naplnenim dat
*/
Entry::Entry(QString title, QString published, QString link, QString id,
    QString author, QString summary, bool alreadyRead) {

    QString format = "yyyy-MM-dd'T'HH:mm:ss'Z'";

    _title = title;
    _published = QDateTime::fromString(published, format);
    _link = link;
    _id = id;
    _alreadyRead = alreadyRead;
    _author = author;
    _summary = summary;
}
