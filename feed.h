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
        void setLabel(QString p_label) { _labelList.append(p_label); }

        void removeLabel(QString p_label) {
            for (int i = 0; i < _labelList.count(); i++) {
                if (p_label == _labelList.at(i)) {
                    _labelList.removeAt(i);
                }
            }
        }

        //nastavi nazev zdroje
        void setTitle(QString p_title) { _title = p_title; }
        //nastavi stitek zdroje
        void setNewestItemTimestamp(int p_newestItemTimestamp) { _newestItemTimestampUsec = p_newestItemTimestamp; }
        //nastavi pocet neprectenych polozek
        void setUnreadCount(int p_count) { _unreadCount = p_count; }
        //znizi pocet neprectenych polozek
        void decrementUnreadCount() { if (_unreadCount > 0) _unreadCount--; }

        QList<Entry*> getEntriesList() {
            return entries;
        }

    private:
        QString _id;
        QString _title;
        QList<QString> _labelList;
        int _unreadCount;
        long _newestItemTimestampUsec;
        long _firstItemMsec;
};

#endif // FEED_H
