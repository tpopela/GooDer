#ifndef FEEDNETWORKING_H
#define FEEDNETWORKING_H

#include "feed.h"

#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QDebug>
#include <QTimer>

class GoogleReader : public QObject {
    Q_OBJECT

    public:
        GoogleReader();

        void getFeeds();
        void getUnreadFeeds();
        void getEntries();
        void getNumberOfEntriesFromFeed(QString, int);
        void addFeed(QString, QString, QString);
        void removeFeed(QString);
        void markEntryAsRead(QString);
        void addFeedLabel(QString, QString);
        void removeFeedLabel(QString, QString);
        void getToken();
        void markFeedAsRead(QString);
        void login();
        void login(QString, QString);

    private:
        QTimer* _timerRefreshSessionID;
        QNetworkCookie* _googleCookie;

        QString _username;
        QString _password;
        QString _sessionID;
        QString _googleToken;
        bool _refreshingSID;

        void setCookie();
        void makeLogin();

    signals:
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
        void signalConnected(bool);

        void shareRawFeeds(QByteArray);
        void shareRawUnreadFeeds(QByteArray);
        void shareRawEntries(QByteArray);

    private slots:
        void parseToken(QNetworkReply*);
        void parseSessionID(QNetworkReply*);
        void parseFeeds(QNetworkReply*);
        void parseUnreadFeeds(QNetworkReply*);
        void parseEntries(QNetworkReply*);
        void parseRespondeMarkFeedAsRead(QNetworkReply*);
        void parseMarkEntryAsRead(QNetworkReply*);
        void parseAddFeedLabel(QNetworkReply*);
        void parseRemoveFeedLabel(QNetworkReply*);
        void parseAddFeed(QNetworkReply*);
        void parseRemoveFeed(QNetworkReply*);
        void refreshSID();
};

#endif // FEEDNETWORKING_H
