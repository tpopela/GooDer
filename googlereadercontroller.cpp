#include "googlereadercontroller.h"

GoogleReaderController::GoogleReaderController() {
    _database = new Database();
    _googleReader = new GoogleReader();
    _firstRun = true;
    _alreadyFetchingNoEntries = false;
    _feedsWithNoEntriesCount = 0;
    _fetchingEntriesFromHistory = false;

    connect(_googleReader, SIGNAL(signalConnected(bool)),
            this, SLOT(statusConnected(bool)));
    connect(_googleReader, SIGNAL(signalStatusMarkFeedAsRead(bool)),
            this, SLOT(statusMarkFeedAsRead(bool)));
    connect(_googleReader, SIGNAL(signalStatusMarkEntryAsRead(bool)),
            this, SLOT(statusMarkEntryAsRead(bool)));
    connect(_googleReader, SIGNAL(signalStatusLogin(bool)),
            this, SLOT(statusLogin(bool)));
    connect(_googleReader, SIGNAL(shareRawFeeds(QByteArray)),
            this, SLOT(rawGetFeeds(QByteArray)));
    connect(_googleReader, SIGNAL(shareRawUnreadFeeds(QByteArray)),
            this, SLOT(rawGetUnreadFeeds(QByteArray)));
    connect(_googleReader, SIGNAL(shareRawEntries(QByteArray)),
            this, SLOT(rawGetEntries(QByteArray)));
    connect(_googleReader, SIGNAL(signalStatusAddFeedLabel(bool)),
            this, SLOT(statusAddFeedLabel(bool)));
    connect(_googleReader, SIGNAL(signalStatusRemoveFeedLabel(bool)),
            this, SLOT(statusRemoveFeedLabel(bool)));
    connect(_googleReader, SIGNAL(signalStatusAddFeed(bool)),
            this, SLOT(statusAddFeed(bool)));
    connect(_googleReader, SIGNAL(signalStatusRemoveFeed(bool)),
            this, SLOT(statusRemoveFeed(bool)));
}

/*!
\brief
*/
int GoogleReaderController::getUnreadCountInLabel(QString label) {
    return _database->getUnreadCountInLabel(label);
}

/*!
\brief
*/
int GoogleReaderController::getUnreadCountInFeed(QString feedId) {
    return _database->getUnreadCountInFeed(feedId);
}

/*!
\brief
*/
void GoogleReaderController::markLabelAsRead(QString label) {

    QList<Feed*> feedsInLabel = _database->getFeedsInLabel(label);

    foreach (Feed* feed, feedsInLabel) {
        _googleReader->markFeedAsRead(feed->getId());
        _database->markFeedAsRead(feed->getId());
    }
}

/*!
\brief
*/
void GoogleReaderController::markFeedAsRead(QString feedId) {

    _googleReader->markFeedAsRead(feedId);
    _database->markFeedAsRead(feedId);
}

/*!
\brief
*/
void GoogleReaderController::markEntryAsRead(QString entryId) {

    _googleReader->markEntryAsRead(entryId);
    _database->markEntryAsRead(entryId);
}

void GoogleReaderController::markAllAsRead()
{
    foreach (Feed* feed, _database->getDatabase())
    {
        if (feed->getUnreadCount() > 0)
            this->markFeedAsRead(feed->getId());
    }
}

/*!
\brief
*/
QList<QString> GoogleReaderController::getLabelsList() {
    return _database->getLabelsList();
}

/*!
\brief
*/
QString GoogleReaderController::getIdForFeed(QString feedName) {
    return _database->getIdForFeed(feedName);
}

/*!
\brief
*/
QString GoogleReaderController::getEntryLink(QString entryId) {
    return _database->getEntryLink(entryId);
}

/*!
\brief
*/
Entry* GoogleReaderController::getEntry(QString entryId) {
    return _database->getEntry(entryId);
}

/*!
\brief
*/
QString GoogleReaderController::getFeedIdForEntry(QString entryId) {
    return _database->getFeedIdForEntry(entryId);
}

void GoogleReaderController::login(QString username, QString password) {

    _googleReader->login(username, password);
}

void GoogleReaderController::login() {

    _googleReader->login();
}

void GoogleReaderController::getFeeds() {

    _googleReader->getFeeds();
}

void GoogleReaderController::rawGetFeeds(QByteArray rawFeeds) {
    _database->addFeeds(rawFeeds);
    this->getUnreadFeeds();
}

void GoogleReaderController::getUnreadFeeds() {

    _googleReader->getUnreadFeeds();
}

void GoogleReaderController::rawGetUnreadFeeds(QByteArray rawFeeds) {
    _database->addUnreadFeeds(rawFeeds);
    this->checkIfFetchEntries();
}

int GoogleReaderController::getTotalUnreadCount() {
    return _database->getTotalUnreadCount();
}

void GoogleReaderController::checkIfFetchEntries() {
    _alreadyFetchingNoEntries = false;
    if (!_firstRun) {
        if (_database->getTotalUnreadCount() > 0) {
            qDebug() << "There are new entries, get them";
            this->getEntries();
        }
        else {
            qDebug() << "There are no new etries";
            return;
        }
    } else {
        qDebug() << "First run of GooDer -> getting all entries";
        _firstRun = false;
        this->getEntries();
    }
}

void GoogleReaderController::getEntries() {
    _googleReader->getEntries();
}

void GoogleReaderController::getSpecifiedNumberOfEntriesFromFeed(QString feedId, int count) {
    _fetchingEntriesFromHistory = true;
    _googleReader->getNumberOfEntriesFromFeed(feedId, count);
}

void GoogleReaderController::fetchFeedsWithNoEntries() {

    _feedsWithNoEntriesCount = 0;
    _alreadyFetchingNoEntries = true;

    foreach (Feed* feed, this->getFeedsDB())
    {
        if (feed->getEntriesList().count() == 0)
        {
            _feedsWithNoEntriesCount++;
            getSpecifiedNumberOfEntriesFromFeed(feed->getId(), 5);
        }
    }
}

void GoogleReaderController::rawGetEntries(QByteArray rawEntries) {
    _database->addEntries(rawEntries);

    if (!_alreadyFetchingNoEntries)
        fetchFeedsWithNoEntries();

    if (_feedsWithNoEntriesCount >= 0)
    {
        if (--_feedsWithNoEntriesCount < 0)
        {
            emit signalStatusGetEntries(true);
        }
        _fetchingEntriesFromHistory = false;
    }

    _firstRun = false;

    if (_fetchingEntriesFromHistory)
    {
        _fetchingEntriesFromHistory = false;
        emit signalStatusGetEntries(true);
    }
}

void GoogleReaderController::addFeedLabel(QString feedId, QString label) {
    _googleReader->addFeedLabel(feedId, label);
    _database->addFeedLabel(feedId, label);
}

void GoogleReaderController::removeFeedLabel(QString feedId, QString label) {
    _googleReader->removeFeedLabel(feedId, label);
    _database->removeFeedLabel(feedId, label);
}

void GoogleReaderController::addFeed(QString address, QString name, QString label) {
    _googleReader->addFeed(address, name, label);
    this->getFeeds();
}

void GoogleReaderController::removeFeed(QString feedId) {
    _googleReader->removeFeed(feedId);
    _database->removeFeed(feedId);
}

Feed* GoogleReaderController::getFeedDB(QString feedId) {
    return _database->getFeed(feedId);
}

QList<Feed*> GoogleReaderController::getFeedsDB() {
    return _database->getDatabase();
}

QList<Feed*> GoogleReaderController::getFeedsInLabelDB(QString label) {
    return _database->getFeedsInLabel(label);
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
