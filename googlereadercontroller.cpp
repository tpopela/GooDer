#include "googlereadercontroller.h"

GoogleReaderController::GoogleReaderController() {
    database = new Database();
    googleReader = new GoogleReader();
    firstRun = true;
    alreadyFetchingNoEntries = false;
    cnt = 0;

    connect(googleReader, SIGNAL(signalConnected(bool)),
            this, SLOT(statusConnected(bool)));
    connect(googleReader, SIGNAL(signalStatusMarkFeedAsRead(bool)),
            this, SLOT(statusMarkFeedAsRead(bool)));
    connect(googleReader, SIGNAL(signalStatusMarkEntryAsRead(bool)),
            this, SLOT(statusMarkEntryAsRead(bool)));
    connect(googleReader, SIGNAL(signalStatusLogin(bool)),
            this, SLOT(statusLogin(bool)));
    connect(googleReader, SIGNAL(shareRawFeeds(QByteArray)),
            this, SLOT(rawGetFeeds(QByteArray)));
    connect(googleReader, SIGNAL(shareRawUnreadFeeds(QByteArray)),
            this, SLOT(rawGetUnreadFeeds(QByteArray)));
    connect(googleReader, SIGNAL(shareRawEntries(QByteArray)),
            this, SLOT(rawGetEntries(QByteArray)));
    connect(googleReader, SIGNAL(signalStatusAddFeedLabel(bool)),
            this, SLOT(statusAddFeedLabel(bool)));
    connect(googleReader, SIGNAL(signalStatusRemoveFeedLabel(bool)),
            this, SLOT(statusRemoveFeedLabel(bool)));
    connect(googleReader, SIGNAL(signalStatusAddFeed(bool)),
            this, SLOT(statusAddFeed(bool)));
    connect(googleReader, SIGNAL(signalStatusRemoveFeed(bool)),
            this, SLOT(statusRemoveFeed(bool)));
}

/*!
\brief
*/
int GoogleReaderController::getUnreadCountInLabel(QString label) {
    return database->getUnreadCountInLabel(label);
}

/*!
\brief
*/
int GoogleReaderController::getUnreadCountInFeed(QString feedName) {
    return database->getUnreadCountInFeed(feedName);
}

/*!
\brief
*/
void GoogleReaderController::markLabelAsRead(QString labelName) {

    QList<QString> feedsInLabel = database->getFeedsInLabel(labelName);

    foreach (QString feedId, feedsInLabel) {
        googleReader->markFeedAsRead(feedId);
        database->markFeedAsRead(feedId);
    }
}

/*!
\brief
*/
void GoogleReaderController::markFeedAsRead(QString feedId) {

    googleReader->markFeedAsRead(feedId);
    database->markFeedAsRead(feedId);
}

/*!
\brief
*/
void GoogleReaderController::markEntryAsRead(QString entryId) {

    googleReader->markEntryAsRead(entryId);
    database->markEntryAsRead(entryId);
}

void GoogleReaderController::markAllAsRead()
{
    foreach (Feed* feed, database->getDatabase())
    {
        if (feed->getUnreadCount() > 0)
            this->markFeedAsRead(feed->getId());
    }
}

/*!
\brief
*/
QList<QString> GoogleReaderController::getLabelsList() {
    return database->getLabelsList();
}

/*!
\brief
*/
QString GoogleReaderController::getIdForFeed(QString feedName) {
    return database->getIdForFeed(feedName);
}

/*!
\brief
*/
QString GoogleReaderController::getEntryLink(QString entryId) {
    return database->getEntryLink(entryId);
}

/*!
\brief
*/
Entry* GoogleReaderController::getEntry(QString entryId) {
    return database->getEntry(entryId);
}

/*!
\brief
*/
QString GoogleReaderController::getFeedIdForEntry(QString entryId) {
    return database->getFeedIdForEntry(entryId);
}

void GoogleReaderController::login(QString username, QString password) {

    googleReader->login(username, password);
}

void GoogleReaderController::login() {

    googleReader->login();
}

void GoogleReaderController::getFeeds() {

    googleReader->getFeeds();
}

void GoogleReaderController::rawGetFeeds(QByteArray rawFeeds) {
    database->addFeeds(rawFeeds);
    this->getUnreadFeeds();
}

void GoogleReaderController::getUnreadFeeds() {

    googleReader->getUnreadFeeds();
}

void GoogleReaderController::rawGetUnreadFeeds(QByteArray rawFeeds) {
    database->addUnreadFeeds(rawFeeds);
    this->checkIfFetchEntries();
}

int GoogleReaderController::getTotalUnreadCount() {
    return database->getTotalUnreadCount();
}

void GoogleReaderController::checkIfFetchEntries() {
    alreadyFetchingNoEntries = false;
    if (!firstRun) {
        if (database->getTotalUnreadCount() > 0) {
            qDebug() << "There are new entries, get them";
            this->getEntries();
        }
        else {
            qDebug() << "There are no new etries";
//            emit signalStatusGetEntries(false);
            return;
        }
    } else {
        qDebug() << "First run of GooDer -> getting all entries";
        firstRun = false;
        this->getEntries();
    }
}

void GoogleReaderController::getEntries() {
    googleReader->getEntries();
}

void GoogleReaderController::getSpecifiedNumberOfEntriesFromFeed(QString feedId, int count) {
    googleReader->getNumberOfEntriesFromFeed(feedId, count);
}

void GoogleReaderController::fetchFeedsWithNoEntries() {

    cnt = 0;
    alreadyFetchingNoEntries = true;

    foreach (Feed* feed, this->getFeedsFromDatabase())
        if (feed->getEntriesList().count() == 0)
        {
            cnt++;
            getSpecifiedNumberOfEntriesFromFeed(feed->getId(), 5);
        }
}

void GoogleReaderController::rawGetEntries(QByteArray rawEntries) {
    database->addEntries(rawEntries);

    if (!alreadyFetchingNoEntries)
        fetchFeedsWithNoEntries();

    if (cnt >= 0)
    {
        if (--cnt < 0)
        {
            emit signalStatusGetEntries(true);
        }
    }

    firstRun = false;
}

void GoogleReaderController::addFeedLabel(QString feedId, QString labelName) {
    googleReader->addFeedLabel(feedId, labelName);
    database->addFeedLabel(feedId, labelName);
}

void GoogleReaderController::removeFeedLabel(QString feedId, QString labelName) {
    googleReader->removeFeedLabel(feedId, labelName);
    database->removeFeedLabel(feedId, labelName);
}

void GoogleReaderController::addFeed(QString address, QString name, QString label) {
    googleReader->addFeed(address, name, label);
    this->getFeeds();
}

void GoogleReaderController::removeFeed(QString feedId) {
    googleReader->removeFeed(feedId);
    database->removeFeed(feedId);
}

QList<Feed*> GoogleReaderController::getFeedsFromDatabase() {
    return database->getDatabase();
}

/******************************************************************************

        SLOTS

******************************************************************************/

void GoogleReaderController::statusAddFeed(bool status) {
    emit signalStatusAddFeed(status);
}

void GoogleReaderController::statusMarkFeedAsRead(bool status) {
    emit signalStatusMarkFeedAsRead(status);
}

void GoogleReaderController::statusMarkEntryAsRead(bool status) {
    emit signalStatusMarkEntryAsRead(status);
}

void GoogleReaderController::statusAddFeedLabel(bool status) {
    emit signalStatusAddFeedLabel(status);
}

void GoogleReaderController::statusRemoveFeedLabel(bool status) {
    emit signalStatusRemoveFeedLabel(status);
}

void GoogleReaderController::statusRemoveFeed(bool status) {
    emit signalStatusRemoveFeed(status);
}

void GoogleReaderController::statusConnected(bool status) {
    emit signalConnected(status);
}

void GoogleReaderController::statusLogin(bool status) {
    emit signalLoginStatus(status);
}
