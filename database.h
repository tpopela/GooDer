#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include "feed.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = 0);

    int getUnreadCountInLabel(QString);
    int getUnreadCountInFeed(QString);
    int getTotalUnreadCount() { return _numberOfNewEntries; }

    void markFeedAsRead(QString);
    void markEntryAsRead(QString);
    void markLabelAsRead(QString);
    void removeFeed(QString);
    void addFeedLabel(QString, QString);
    void removeFeedLabel(QString, QString);
    void setNumberOfNewEntries(QString, int);
    void addFeeds(QByteArray);
    void addEntries(QByteArray);
    void addUnreadFeeds(QByteArray);

    QString getIdForFeed(QString);
    QString getEntryLink(QString);
    QString getFeedIdForEntry(QString);

    Entry* getEntry(QString);

    QList<Feed*> getDatabase() { return _storage; }
    QList<QString> getFeedsInLabel(QString);
    QList<QString> getLabelsList();

private:
    int _numberOfNewEntries;

    QList<Feed*> _storage;

};

#endif // DATABASE_H
