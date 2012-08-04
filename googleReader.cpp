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
    googleCookie = new QNetworkCookie();
    // Session ID expires in about 20 minutes
    timerRefreshSID = new QTimer(this);

    connect(timerRefreshSID, SIGNAL(timeout()),
            this, SLOT(refreshSID()));
}

/******************************************************************************

  LOGIN TO GOOGLE READER

******************************************************************************/

/*!
\brief Login to Google Reader
*/
void GoogleReader::login(QString p_username, QString p_password) {
    qDebug() << "GoogleReader::login(username, password)";

    this->username = p_username;
    this->password = p_password;

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
    this->login();
}

/*!
\brief Makes actual login
*/
void GoogleReader::makeLogin() {

    if (this->username.isEmpty() && this->password.isEmpty()) {
        return;
    }

    qDebug() << "Login with username: " << this->username;
    QNetworkAccessManager* connectionLogin = new QNetworkAccessManager(this);

    QString url = "https://www.google.com/accounts/ClientLogin";
    QString postData = QString("service=reader&Email=%1&Passwd=%2&Source=GooDer").arg(this->username, this->password);
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
                qDebug() << "Bad username or password";

            qDebug() << reply->readAll();
            emit signalConnected(false);
        }
        return;
    }

    QString SIDReply = reply->readAll();
    if (SIDReply.contains("Auth=")) {
        int startOfSID = SIDReply.indexOf("Auth=")+5;
        QString tempSID = SIDReply.mid(startOfSID);
        sessionID = tempSID.left(tempSID.length()-1);
        qDebug() << "Session ID: " << sessionID;

        this->getToken();
    }
}

/*!
\brief Cookie setup
*/
void GoogleReader::setCookie() {
    qDebug() << "GoogleReader::setCookie()";

    QString strTokenRequest = "GoogleLogin auth=";
    strTokenRequest.append(sessionID);

    googleCookie->setName("Authorization");
    googleCookie->setValue(strTokenRequest.toAscii());
}

/*!
\brief Get Google Token
*/
void GoogleReader::getToken() {
    qDebug() << "GoogleReader::getToken()";

    this->setCookie();

    QNetworkRequest tokenRequest = QNetworkRequest(QUrl("http://www.google.com/reader/api/0/token"));
    tokenRequest.setRawHeader("Authorization", googleCookie->value());
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
    googleToken = reply->readAll();
    qDebug() << "Token: " << googleToken;

    emit signalConnected(true);

    // Session ID expires in about 20 minutes
    timerRefreshSID->start(1820000);

    emit signalStatusLogin(true);
}

/******************************************************************************

  GET FEEDS

******************************************************************************/

/*!
\brief Get list of subscribed feeds
*/
void GoogleReader::getFeeds() {
    qDebug() << "GoogleReader::getFeeds()";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetFeeds = new QNetworkAccessManager(this);

    QString feedUrl = QString("http://www.google.com/reader/api/0/subscription/list?output=xml");
    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", googleCookie->value());

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

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetUnreadFeeds = new QNetworkAccessManager(this);
    QString unixTime = QString::number(getUnixTime());

    QString feedUrl = QString("http://www.google.com/reader/api/0/unread-count?allcomments=true&output=xml&ck=%1&client=googleReader").arg(unixTime);

    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", googleCookie->value());

    managerGetUnreadFeeds->get(getRequest);

    connect(managerGetUnreadFeeds, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseUnreadFeeds(QNetworkReply*)));
}

/*!
\brief Parse list with subscribed feeds (and with number of new entries in feeds)
*/
void GoogleReader::parseUnreadFeeds(QNetworkReply* source) {
    qDebug() << "GoogleReader::ParseUnreadFeeds()";

    source->deleteLater();

    if (source->error() != QNetworkReply::NoError) {
        qDebug() << source->readAll();
        emit signalStatusGetUnreadFeeds(false);
        return;
    }

    emit signalStatusGetUnreadFeeds(true);
    emit shareRawUnreadFeeds(source->readAll());
}

/******************************************************************************

  GET ENTRIES

******************************************************************************/

/*!
\brief Get list of all entries
*/
void GoogleReader::getEntries() {
    qDebug() << "GoogleReader::getEntries()";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QString feedUrl;
    int numberOfEntries = 1000;

    QNetworkAccessManager* managerGetEntriesList = new QNetworkAccessManager(this);
    feedUrl = QString("http://www.google.com/reader/atom/user/-/state/com.google/reading-list?n=%1").arg(numberOfEntries);

    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", googleCookie->value());

    connect(managerGetEntriesList, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseEntries(QNetworkReply*)));

    managerGetEntriesList->get(getRequest);
}

/*!
\brief Get specified number of entries from specified feed
*/
void GoogleReader::getNumberOfEntriesFromFeed(QString p_feedId, int p_count) {
    qDebug() << "GoogleReader::getAllEntriesFromFeed(" << p_feedId << "," << p_count << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager* managerGetAllEntriesFromFeed = new QNetworkAccessManager(this);

    QString feedUrl = QString("http://www.google.com/reader/atom/feed/%1?n=%2").arg(p_feedId, QString::number(p_count));
    QNetworkRequest getRequest= QNetworkRequest(QUrl(feedUrl));
    getRequest.setRawHeader("Authorization", googleCookie->value());

    connect(managerGetAllEntriesFromFeed, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseEntries(QNetworkReply*)));

    managerGetAllEntriesFromFeed->get(getRequest);
}

/*!
\brief Parse entries
*/
void GoogleReader::parseEntries(QNetworkReply *source) {
    qDebug() << "GoogleReader::parseEntries()";

    source->deleteLater();

    if (source->error() != QNetworkReply::NoError) {
        qDebug() << source->readAll();
        emit signalStatusGetEntries(false);
    }

    emit signalStatusGetEntries(true);
    emit shareRawEntries(source->readAll());
}

/******************************************************************************

    MANIPULATING WITH FEEDS AND ENTRIES

******************************************************************************/

/*!
\brief Mark feeds as read
*/
void GoogleReader::markFeedAsRead(QString p_id_stream) {
    qDebug() << "GoogleReader->markFeedAsRead(" << p_id_stream << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerMarkAllAsRead = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/mark-all-as-read";
    QString unixTimeMS = QString::number((getUnixTime()));
    QString postData = QString("s=feed/%1&ts=%2&T=%3").arg(QUrl::toPercentEncoding(p_id_stream), unixTimeMS, googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerMarkAllAsRead, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRespondeMarkFeedAsRead(QNetworkReply*)));

    managerMarkAllAsRead->post(postRequest, postData.toAscii());
}

/*!
\brief Parses respond for marking feed as read
*/
void GoogleReader::parseRespondeMarkFeedAsRead(QNetworkReply* p_reply) {
    qDebug() << "GoogleReader::parseRespondeMarkFeedAsRead()";

    p_reply->deleteLater();
    if (p_reply->readAll() != "OK") {
        qDebug() << p_reply->readAll();
        emit signalStatusMarkFeedAsRead(false);
        return;
    }

    emit signalStatusMarkFeedAsRead(true);
}

/*!
\brief Marks entry (p_id) as read
*/
void GoogleReader::markEntryAsRead(QString p_id) {
    qDebug() << "GoogleReader::markEntryAsRead(" << p_id << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerMarkEntryAsRead = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/edit-tag?client=GooDer";
    QString postData = QString("i=%1&a=user/-/state/com.google/read&ac=edit&T=%2").arg(p_id, googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerMarkEntryAsRead, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseMarkEntryAsRead(QNetworkReply *)));

    managerMarkEntryAsRead->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for marking entry as read
*/
void GoogleReader::parseMarkEntryAsRead(QNetworkReply* p_reply) {
    p_reply->deleteLater();

    if (p_reply->readAll() != "OK") {
        qDebug() << "Error during marking entry as read";
        qDebug() << p_reply->readAll();
        emit signalStatusMarkEntryAsRead(false);
        return;
    }

    emit signalStatusMarkEntryAsRead(true);
}

/*!
\brief Adds label for feed
*/
void GoogleReader::addFeedLabel(QString p_id, QString p_label) {
    qDebug() << "GoogleReader::addFeedLabel(" << p_id << "," << p_label << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerAddLabel = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=edit&a=user/-/label/%2&T=%3").arg(p_id, p_label, googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerAddLabel, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseAddFeedLabel(QNetworkReply *)));

    managerAddLabel->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for adding feed label request
*/
void GoogleReader::parseAddFeedLabel(QNetworkReply* p_reply) {
    qDebug() << "GoogleReader::parseAddFeedLabel()";

    p_reply->deleteLater();

    if (p_reply->readAll() != "OK") {
        qDebug() << p_reply->readAll();
        emit signalStatusAddFeedLabel(false);
        return;
    }

    emit signalStatusAddFeedLabel(true);
}

/*!
\brief Remove label from feed
*/
void GoogleReader::removeFeedLabel(QString p_id, QString p_label) {
    qDebug() << "GoogleReader::removeFeedLabel(" << p_id << "," << p_label << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QNetworkAccessManager * managerRemoveFeedLabel = new QNetworkAccessManager(this);

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=edit&r=user/-/label/%2&T=%3").arg(p_id, p_label, googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(managerRemoveFeedLabel, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRemoveFeedLabel(QNetworkReply *)));

    managerRemoveFeedLabel->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for removing feed label request
*/
void GoogleReader::parseRemoveFeedLabel(QNetworkReply* p_reply) {
    qDebug() << "GoogleReader::parseRemoveFeedLabel()";

    p_reply->deleteLater();

    if (p_reply->readAll() != "OK") {
        qDebug() << p_reply->readAll();
        emit signalStatusRemoveFeedLabel(false);
        return;
    }

    emit signalStatusRemoveFeedLabel(true);
}

/*!
\brief Subscribe new feed
*/
void GoogleReader::addFeed(QString p_address, QString p_name, QString p_label) {
    qDebug() << "GoogleReader::addFeed(" << p_address << "," << p_name << "," << p_label << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    if (p_address.left(7) != "http://")
        p_address.insert(0, "http://");

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = "";

    if (p_name.isEmpty() && p_label.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&T=%2").arg(p_address, googleToken);
    } else if (!p_name.isEmpty() && p_label.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&t=%2&T=%3").arg(p_address, p_name, googleToken);
    } else if (!p_name.isEmpty() && !p_label.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&t=%2&a=user/-/label/%3&T=%4").arg(p_address, p_name, p_label, googleToken);
    } else if (p_name.isEmpty() && !p_label.isEmpty()) {
        postData = QString("s=feed/%1&ac=subscribe&a=user/-/label/%2&T=%3").arg(p_address, p_label, googleToken);
    }

    QNetworkAccessManager* feedConnection = new QNetworkAccessManager(this);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    connect(feedConnection, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseAddFeed(QNetworkReply *)));

    feedConnection->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for subscribing new feed
*/
void GoogleReader::parseAddFeed(QNetworkReply* p_reply) {
    qDebug() << "GoogleReader::parseAddFeed()";

    p_reply->deleteLater();

    if (p_reply->readAll() != "OK") {
        qDebug() << "Error during add feed";
        qDebug() << p_reply->readAll();
        emit signalStatusAddFeed(false);
        return;
    }

    emit signalStatusAddFeed(true);
}

/*!
\brief Remove feed from subscribe list
*/
void GoogleReader::removeFeed(QString p_id) {
    qDebug() << "GoogleReader::removeFeed(" << p_id << ")";

    if (googleCookie == NULL) {
        emit signalConnected(false);
        this->login();
    }

    QString url = "http://www.google.com/reader/api/0/subscription/edit?client=GooDer";
    QString postData = QString("s=feed/%1&ac=unsubscribe&T=%2").arg(p_id, googleToken);

    QNetworkRequest postRequest = QNetworkRequest(QUrl(url));
    postRequest.setRawHeader("Authorization", googleCookie->value());
    postRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkAccessManager * managerRemoveFeed = new QNetworkAccessManager(this);

    connect(managerRemoveFeed, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseRemoveFeed(QNetworkReply *)));

    managerRemoveFeed->post(postRequest, postData.toAscii());
}

/*!
\brief Parses responde for removing feed from subscribe list
*/
void GoogleReader::parseRemoveFeed(QNetworkReply* p_reply) {
    qDebug() << "GoogleReader::parseRemoveFeed()";

    p_reply->deleteLater();

    if (p_reply->readAll() != "OK") {
        qDebug() << p_reply->readAll();
        emit signalStatusRemoveFeed(false);
        return;
    }

    emit signalStatusRemoveFeed(true);
}
