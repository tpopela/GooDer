#ifndef FEED_H
#define FEED_H

#include "entry.h"

#include <QObject>

class Feed : public QObject {

    public:
        QList<Entry*> entries;

        Feed();
        Feed(QString p_id, QString p_title);
        Feed(QString p_id, QString p_title, QList<QString> p_label, QString p_firstItemMsec);

        //navrati ID zdroje
        QString getId() { return id; }
        //navrati nazev zdroje
        QString getTitle() { return title; }
        //navrati stitek pokud je zdroj oznacen
        QList<QString> getLabel() { return labelList; }
        //navrati pocet neprectenych polozek ve zdroji
        int getUnreadCount() { return unreadCount; }

        long getFirstItemTime() { return firstItemMsec; }

        //nastavi stitek zdroje
        void setLabel(QString p_label) { labelList.append(p_label); }

        void removeLabel(QString p_label) {
            for (int i = 0; i < labelList.count(); i++) {
                if (p_label == labelList.at(i)) {
                    labelList.removeAt(i);
                }
            }
        }

        //nastavi nazev zdroje
        void setTitle(QString p_title) { title = p_title; }
        //nastavi stitek zdroje
        void setNewestItemTimestamp(int p_newestItemTimestamp) { newestItemTimestampUsec = p_newestItemTimestamp; }
        //nastavi pocet neprectenych polozek
        void setUnreadCount(int p_count) { unreadCount = p_count; }
        //znizi pocet neprectenych polozek
        void decrementUnreadCount() { if (unreadCount > 0) unreadCount--; }

        QList<Entry*> getEntriesList() {
            return entries;
        }

    private:
        QString id;
        QString title;
        QList<QString> labelList;
        int unreadCount;
        long newestItemTimestampUsec;
        long firstItemMsec;
};

#endif // FEED_H
