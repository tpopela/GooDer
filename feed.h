#ifndef FEED_H
#define FEED_H

#include "entry.h"

#include <QObject>

class Feed : public QObject {

    public:
        QList<Entry*> entries;

        Feed();
        Feed(QString, QString);
        Feed(QString, QString, QList<QString>, QString);

        //navrati ID zdroje
        QString getId() { return _id; }
        //navrati nazev zdroje
        QString getTitle() { return _title; }
        //navrati stitek pokud je zdroj oznacen
        QList<QString> getLabel() { return _labelList; }
        //navrati pocet neprectenych polozek ve zdroji
        int getUnreadCount() { return _unreadCount; }

        long getFirstItemTime() { return _firstItemMsec; }

        //nastavi stitek zdroje
        void setLabel(QString label) { _labelList.append(label); }

        //nastavi nazev zdroje
        void setTitle(QString title) { _title = title; }
        //nastavi stitek zdroje
        void setNewestItemTimestamp(int newestItemTimestamp) { _newestItemTimestampUsec = newestItemTimestamp; }
        //nastavi pocet neprectenych polozek
        void setUnreadCount(int count) { _unreadCount = count; }
        //znizi pocet neprectenych polozek
        void decrementUnreadCount() { if (_unreadCount > 0) _unreadCount--; }

        void removeLabel(QString);

        QList<Entry*> getEntriesList() { return entries; }

    private:
        QString _id;
        QString _title;
        QList<QString> _labelList;
        int _unreadCount;
        long _newestItemTimestampUsec;
        long _firstItemMsec;
};

#endif // FEED_H
