#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QDate>

class Entry : public QObject
{
    public:
        Entry();
        Entry(QString, QString, QString, QString, QString, bool);
        Entry(QString, QString, QString, QString, QString, QString, bool);
        //navrati titulek polozky
        QString getTitle() { return _title; }
        //navrati adresu polozky
        QString getLink() { return _link; }
        //navrati autora polozky
        QString getAuthor() { return _author; }
        //navrati ID polozky
        QString getId() { return _id; }
        //navrati datum publikovani
        QDateTime getPublishedDate() { return _published; }

        QString getSummary() { return _summary; }
        //navrati jestli je polozka oznacena jako prectena
        bool isRead() { return _alreadyRead; }
        //nastavi polozku jako prectenou
        void setAsRead() { _alreadyRead = true; }

    private:
        QDateTime _published;

        bool _alreadyRead;

        QString _title;
        QString _link;
        QString _id;
        QString _author;
        QString _summary;
};

#endif // ENTRY_H
