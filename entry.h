#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QDate>

class Entry : public QObject
{
    public:
        Entry();
        Entry(QString p_title, QString p_published, QString p_link, QString p_id, QString p_author, bool p_read);
        Entry(QString p_title, QString p_published, QString p_link, QString p_id, QString p_author, QString p_summary, bool p_read);
        //navrati titulek polozky
        QString getTitle() { return title; }
        //navrati adresu polozky
        QString getLink() { return link; }
        //navrati autora polozky
        QString getAuthor() { return author; }
        //navrati ID polozky
        QString getId() { return id; }
        //navrati datum publikovani
        QDateTime getPublishedDate() { return published; }

        QString getSummary() { return summary; }
        //navrati jestli je polozka oznacena jako prectena
        bool isRead() { return f_read; }
        //nastavi polozku jako prectenou
        void setAsRead() { f_read = true; }

    private:
        bool f_read;
        QString title;
        QDateTime published;
        QString link;
        QString id;
        QString author;
        QString summary;
};

#endif // ENTRY_H
