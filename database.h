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
    void markFeedAsRead(QString);
    void markEntryAsRead(QString);
    void markLabelAsRead(QString);
    void removeFeed(QString);
    void addFeedLabel(QString, QString);
    void removeFeedLabel(QString, QString);
    QList<QString> getFeedsInLabel(QString);
    QList<QString> getLabelsList();
    void setNumberOfNewEntries(QString, int);
    QString getIdForFeed(QString);
    QString getEntryLink(QString);
    Entry* getEntry(QString);
    QString getFeedIdForEntry(QString);
    void addFeeds(QByteArray);
    void addEntries(QByteArray);
    void addUnreadFeeds(QByteArray);
    int getTotalUnreadCount() { return numberOfNewEntries; }
    QList<Feed*> getDatabase() { return storage; }

private:
    int numberOfNewEntries;

    QList<Feed*> storage;


signals:

public slots:

};

#endif // DATABASE_H
