#ifndef GOOGLEREADERCONTROLLER_H
#define GOOGLEREADERCONTROLLER_H

#include <QObject>
#include "database.h"
#include "googleReader.h"

class GoogleReaderController : public QObject
{
    Q_OBJECT
public:
    GoogleReaderController();
    int getUnreadCountInLabel(QString);
    int getUnreadCountInFeed(QString);
    void markLabelAsRead(QString);
    void markFeedAsRead(QString);
    void markEntryAsRead(QString);
    void markAllAsRead();
    void addFeedLabel(QString,QString);
    void removeFeedLabel(QString, QString);
    void addFeed(QString, QString, QString);
    void removeFeed(QString);
    QList<QString> getLabelsList();
    QList<Feed*> getFeedsFromDatabase();
    QString getIdForFeed(QString);
    QString getEntryLink(QString);
    Entry* getEntry(QString);
    QString getFeedIdForEntry(QString);
    void login(QString, QString);
    void login();
    void getFeeds();
    void getUnreadFeeds();
    void checkIfFetchEntries();
    void getEntries();
    void getSpecifiedNumberOfEntriesFromFeed(QString, int);
    int getTotalUnreadCount();

private:
    Database* database;
    GoogleReader* googleReader;
    bool firstRun;
    bool alreadyFetchingNoEntries;
    int cnt;

signals:
    void signalConnected(bool);
    void signalLoginStatus(bool);

    void signalStatusLogin(bool);
    void signalStatusGetFeeds(bool);
    void signalStatusGetEntries(bool);
    void signalStatusGetUnreadFeeds(bool);
    void signalStatusMarkFeedAsRead(bool);
    void signalStatusMarkEntryAsRead(bool);
    void signalStatusAddFeedLabel(bool);
    void signalStatusRemoveFeedLabel(bool);
    void signalStatusAddFeed(bool);
    void signalStatusRemoveFeed(bool);

public slots:

private slots:
    void rawGetFeeds(QByteArray);
    void rawGetUnreadFeeds(QByteArray);
    void rawGetEntries(QByteArray);
    void fetchFeedsWithNoEntries();

    void statusLogin(bool);
//    void statusGetFeeds(bool);
//    void statusGetEntries(bool);
//    void statusGetUnreadFeeds(bool);
    void statusMarkFeedAsRead(bool);
    void statusMarkEntryAsRead(bool);
    void statusAddFeedLabel(bool);
    void statusRemoveFeedLabel(bool);
    void statusAddFeed(bool);
    void statusRemoveFeed(bool);

    void statusConnected(bool);
};

#endif // GOOGLEREADERCONTROLLER_H
