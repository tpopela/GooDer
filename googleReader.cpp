#include "googleReader.h"
#include "functions.h"

#include <QDebug>

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookie>
#include <QUrl>

/*!
\brief Vychozi konstruktor
*/
GoogleReader::GoogleReader() {
    _googleCookie = new QNetworkCookie();
    // Session ID expires in about 20 minutes
    _timerRefreshSessionID = new QTimer(this);
    _refreshingSID = false;

    connect(_timerRefreshSessionID, SIGNAL(timeout()),
            this, SLOT(refreshSID()));
}

/******************************************************************************

  LOGIN TO GOOGLE READER

******************************************************************************/

/*!
\brief Login to Google Reader
*/
void GoogleReader::login(QString username, QString password) {
    qDebug() << "GoogleReader::login(username, password)";

    this->_username = username;
    this->_password = password;

    this->makeLogin();
}

/*!
\brief Login to Google Reader
*/
void GoogleReader::login() {
    qDebug() << "GoogleReader::login()";

    this->makeLogin();
}

void GoogleReader::refreshSID() {
    qDebug() << "GoogleReader::refreshSID()";

    _refreshingSID = true;
    this->makeLogin();
}

/*!
\brief Makes actual login
*/
void GoogleReader::makeLogin() {

    if (this->_username.isEmpty() && this->_password.isEmpty()) {
        return;
    }

    qDebug() << "Login with username: " << this->_username;
    QNetworkAccessManager* connectionLogin = new QNetworkAccessManager(this);

    QString url = "https://www.google.com/accounts/ClientLogin";
    QString postData = QString("service=reader&Email=%1&Passwd=%2&Source=GooDer").arg(this->_username, this->_password);
    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));

    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    connectionLogin->post(postRequest, postData.toAscii());

    connect(connectionLogin, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(parseSessionID(QNetworkReply*)));
}

/*!
\brief V prijmute odpovedi hleda Session ID a vytvori cookie, ktera je potreba pro vykonavani pozadavku.
\brief Parse session ID from reply
*/
void GoogleReader::parseSessionID(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseSessionID()";

    reply->deleteLater();
    // check if response contains errors
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() == QNetworkReply::HostNotFoundError) {
            qDebug() << "Host not found -> maybe offline?";
            emit signalConnected(false);
        }
        else {
            QString serverReply = reply->errorString();
            if (serverReply.contains("Forbidden"))
            {
                qDebug() << "Bad username or password";
            }

            qDebug() << reply->readAll();
            emit signalConnected(false);
        }
        return;
    }

    QString SIDReply = reply->readAll();
    if (SIDReply.contains("Auth=")) {
        int startOfSID = SIDReply.indexOf("Auth=")+5;
        QString tempSID = SIDReply.mid(startOfSID);
        _sessionID = tempSID.left(tempSID.length()-1);
        qDebug() << "Session ID: " << _sessionID;

        this->getToken();
    }
}

/*!
\brief Cookie setup
*/
void GoogleReader::setCookie() {
    qDebug() << "GoogleReader::setCookie()";

    QString strTokenRequest = "GoogleLogin auth=";
    strTokenRequest.append(_sessionID);

    _googleCookie->setName("Authorization");
    _googleCookie->setValue(strTokenRequest.toAscii());
}

/*!
\brief Get Google Token
*/
void GoogleReader::getToken() {
    qDebug() << "GoogleReader::getToken()";

    this->setCookie();

    QNetworkRequest tokenRequest = QNetworkRequest(QUrl("http://www.google.com/reader/api/0/token"));
    tokenRequest.setRawHeader("Authorization", _googleCookie->value());
    QNetworkAccessManager* connectionToken = new QNetworkAccessManager(this);

    connect(connectionToken, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseToken(QNetworkReply*)));

    connectionToken->get(tokenRequest);
}

/*!
\brief Parse token from reply
*/
void GoogleReader::parseToken(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseToken";

    reply->deleteLater();

    //zkontrolujeme odpoved na chyby
    if (reply->error() != QNetworkReply::NoError ) {
        qDebug() << reply->readAll();
        emit signalStatusLogin(false);
        emit signalConnected(false);
        return;
    }

    //ulozime si token
    _googleToken = reply->readAll();
    qDebug() << "Token: " << _googleToken;

    if (!_refreshingSID) {
        emit signalConnected(true);

        // Session ID expires in about 20 minutes
        _timerRefreshSessionID->start(1820000);

        emit signalStatusLogin(true);
    }

    _refreshingSID = false;
}

/******************************************************************************

  GET FEEDS

******************************************************************************/

/*!
\brief Get list of subscribed feeds
*/
void GoogleReader::getFeeds() {
    qDebug() << "GoogleReader::getFeeds()";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetFeeds = new QNetworkAccessManager(this);

    QString feedUrl = QString("http://www.google.com/reader/api/0/subscription/list?output=xml");
    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", _googleCookie->value());

    connect(managerGetFeeds, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseFeeds(QNetworkReply*)));

    managerGetFeeds->get(getRequest);
}

/*!
\brief Z odpovedi od serveru ziska seznam zdroju s neprectenymi polozkami
*/
void GoogleReader::parseFeeds(QNetworkReply* source) {
    qDebug() << "GoogleReader::parseFeeds()";

    source->deleteLater();

    if (source->error() != QNetworkReply::NoError) {
        qDebug() << source->readAll();
        emit signalStatusGetFeeds(false);
        return;
    }

    emit signalStatusGetFeeds(true);
    emit shareRawFeeds(source->readAll());
}

/*!
\brief Ziska seznam zdroju, ve kterych jsou neprectene polozky
*/
void GoogleReader::getUnreadFeeds() {
    qDebug() << "GoogleReader::getUnreadFeeds()";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetUnreadFeeds = new QNetworkAccessManager(this);
    QString unixTime = QString::number(getUnixTime());

    QString feedUrl = QString("http://www.google.com/reader/api/0/unread-count?allcomments=true&output=xml&ck=%1&client=googleReader").arg(unixTime);

    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", _googleCookie->value());

    managerGetUnreadFeeds->get(getRequest);

    connect(managerGetUnreadFeeds, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseUnreadFeeds(QNetworkReply*)));
}

/*!
\brief Parse list with subscribed feeds (and with number of new entries in feeds)
*/
void GoogleReader::parseUnreadFeeds(QNetworkReply* reply) {
    qDebug() << "GoogleReader::ParseUnreadFeeds()";

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->readAll();
        emit signalStatusGetUnreadFeeds(false);
        return;
    }

    emit signalStatusGetUnreadFeeds(true);
    emit shareRawUnreadFeeds(reply->readAll());
}

/******************************************************************************

  GET ENTRIES

******************************************************************************/

/*!
\brief Get list of all entries
*/
void GoogleReader::getEntries() {
    qDebug() << "GoogleReader::getEntries()";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QString feedUrl;
    int numberOfEntries = 1000;

    QNetworkAccessManager* managerGetEntriesList = new QNetworkAccessManager(this);
    feedUrl = QString("http://www.google.com/reader/atom/user/-/state/com.google/reading-list?n=%1").arg(numberOfEntries);

    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", _googleCookie->value());

    connect(managerGetEntriesList, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseEntries(QNetworkReply*)));

    managerGetEntriesList->get(getRequest);
}

/*!
\brief Get specified number of entries from specified feed
*/
void GoogleReader::getNumberOfEntriesFromFeed(QString feedId, int count) {
    qDebug() << "GoogleReader::getAllEntriesFromFeed(" << feedId << "," << count << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetAllEntriesFromFeed = new QNetworkAccessManager(this);

    QString feedUrl = QString("http://www.google.com/reader/atom/feed/%1?n=%2").arg(feedId, QString::number(count));
    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", _googleCookie->value());

    connect(managerGetAllEntriesFromFeed, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseEntries(QNetworkReply*)));

    managerGetAllEntriesFromFeed->get(getRequest);
}

/*!
\brief Parse entries
*/
void GoogleReader::parseEntries(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseEntries()";

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->readAll();
        emit signalStatusGetEntries(false);
    }

    emit shareRawEntries(reply->readAll());
    emit signalStatusGetEntries(true);
}

/******************************************************************************

    MANIPULATING WITH FEEDS AND ENTRIES

******************************************************************************/

/*!
\brief Mark feeds as read
*/
void GoogleReader::markFeedAsRead(QString feedId) {
    qDebug() << "GoogleReader::markFeedAsRead(" << feedId << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerMarkAllAsRead = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/mark-all-as-read";
    QString unixTimeMS = QString::number((getUnixTime()));
    QString postData = QString("s=feed/%1&ts=%2&T=%3").arg(QUrl::toPercentEncoding(feedId), unixTimeMS, _googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Id", feedId.toAscii());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerMarkAllAsRead, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRespondeMarkFeedAsRead(QNetworkReply*)));

    managerMarkAllAsRead->post(postRequest, postData.toAscii());
}

/*!
\brief Parses respond for marking feed as read
*/
void GoogleReader::parseRespondeMarkFeedAsRead(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseRespondeMarkFeedAsRead()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during marking feed as read";
        qDebug() << strReply;
        refreshSID();
        markFeedAsRead(reply->request().rawHeader("Id"));
        return;
    }

    emit signalStatusMarkFeedAsRead(true);
}

/*!
\brief Marks entry as read
*/
void GoogleReader::markEntryAsRead(QString entryId) {
    qDebug() << "GoogleReader::markEntryAsRead(" << entryId << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerMarkEntryAsRead = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/edit-tag?client=GooDer";
    QString postData = QString("i=%1&a=user/-/state/com.google/read&ac=edit&T=%2").arg(entryId, _googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Id", entryId.toAscii());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerMarkEntryAsRead, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseMarkEntryAsRead(QNetworkReply *)));

    managerMarkEntryAsRead->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for marking entry as read
*/
void GoogleReader::parseMarkEntryAsRead(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseMarkEntryAsRead()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during marking entry as read";
        qDebug() << strReply;
        refreshSID();
        markEntryAsRead(reply->request().rawHeader("Id"));
        return;
    }

    emit signalStatusMarkEntryAsRead(true);
}

/*!
\brief Adds label for feed
*/
void GoogleReader::addFeedLabel(QString feedId, QString feedLabel) {
    qDebug() << "GoogleReader::addFeedLabel(" << feedId << "," << feedLabel << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerAddLabel = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=edit&a=user/-/label/%2&T=%3").arg(feedId, feedLabel, _googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerAddLabel, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseAddFeedLabel(QNetworkReply *)));

    managerAddLabel->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for adding feed label request
*/
void GoogleReader::parseAddFeedLabel(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseAddFeedLabel()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during adding label to feed";
        qDebug() << strReply;
        emit signalStatusAddFeedLabel(false);
        return;
    }

    emit signalStatusAddFeedLabel(true);
}

/*!
\brief Remove label from feed
*/
void GoogleReader::removeFeedLabel(QString feedId, QString feedLabel) {
    qDebug() << "GoogleReader::removeFeedLabel(" << feedId << "," << feedLabel << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerRemoveFeedLabel = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=edit&r=user/-/label/%2&T=%3").arg(feedId, feedLabel, _googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerRemoveFeedLabel, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRemoveFeedLabel(QNetworkReply *)));

    managerRemoveFeedLabel->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for removing feed label request
*/
void GoogleReader::parseRemoveFeedLabel(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseRemoveFeedLabel()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during removing label from feed";
        qDebug() << strReply;
        emit signalStatusRemoveFeedLabel(false);
        return;
    }

    emit signalStatusRemoveFeedLabel(true);
}

/*!
\brief Subscribe new feed
*/
void GoogleReader::addFeed(QString feedAddress, QString feedName, QString feedLabel) {
    qDebug() << "GoogleReader::addFeed(" << feedAddress << "," << feedName << "," << feedLabel << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    if (feedAddress.left(7) != "http://")
        feedAddress.insert(0, "http://");

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = "";

    if (feedName.isEmpty() && feedLabel.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&T=%2").arg(feedAddress, _googleToken);
    } else if (!feedName.isEmpty() && feedLabel.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&t=%2&T=%3").arg(feedAddress, feedName, _googleToken);
    } else if (!feedName.isEmpty() && !feedLabel.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&t=%2&a=user/-/label/%3&T=%4").arg(feedAddress, feedName, feedLabel, _googleToken);
    } else if (feedName.isEmpty() && !feedLabel.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&a=user/-/label/%2&T=%3").arg(feedAddress, feedLabel, _googleToken);
    }

    QNetworkAccessManager* feedConnection = new QNetworkAccessManager(this);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(feedConnection, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseAddFeed(QNetworkReply *)));

    feedConnection->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for subscribing new feed
*/
void GoogleReader::parseAddFeed(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseAddFeed()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during add feed";
        qDebug() << strReply;
        emit signalStatusAddFeed(false);
        return;
    }

    emit signalStatusAddFeed(true);
}

/*!
\brief Remove feed from subscribe list
*/
void GoogleReader::removeFeed(QString feedId) {
    qDebug() << "GoogleReader::removeFeed(" << feedId << ")";

    if (_googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=unsubscribe&T=%2").arg(feedId, _googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", _googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkAccessManager * managerRemoveFeed = new QNetworkAccessManager(this);

    connect(managerRemoveFeed, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRemoveFeed(QNetworkReply *)));

    managerRemoveFeed->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for removing feed from subscribe list
*/
void GoogleReader::parseRemoveFeed(QNetworkReply* reply) {
    qDebug() << "GoogleReader::parseRemoveFeed()";

    reply->deleteLater();

    QString strReply = reply->readAll();

    if (strReply != "OK") {
        qDebug() << "Error during removing feed";
        qDebug() << strReply;
        emit signalStatusRemoveFeed(false);
        return;
    }

    emit signalStatusRemoveFeed(true);
}
