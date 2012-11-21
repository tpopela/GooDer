#include "database.h"

#include <QXmlStreamReader>
#include <QDebug>

Database::Database(QObject *parent) :
    QObject(parent)
{
}

/*!
\brief
*/
int Database::getUnreadCountInLabel(QString label) {
    bool flagLabel = false;
    int sum = 0;
    foreach (Feed* feed, _storage) {
        QList<QString> labelList = feed->getLabel();
        foreach (QString feedLabel, labelList) {
           if (feedLabel == label) {
               flagLabel = true;
               sum += feed->getUnreadCount();
           }
        }
    }
    if (flagLabel) return sum;
    else return -1;
}

void Database::removeFeed(QString feedId) {
    for (int i = 0; i < _storage.count(); i++) {
        if (_storage.at(i)->getId() == feedId) {
            _storage.removeAt(i);
        }
    }
}

void Database::markFeedAsRead(QString feedId) {
    foreach (Feed* feed, _storage) {
        if (feed->getId() == feedId) {
            foreach (Entry* entry, feed->getEntriesList()) {
                if (!entry->isRead()) {
                    entry->setAsRead();
                    feed->decrementUnreadCount();
                    _numberOfNewEntries--;
                }
            }
            break;
        }
    }
}

void Database::markEntryAsRead(QString entryId) {
    foreach (Feed* feed, _storage) {
        foreach (Entry* entry, feed->getEntriesList()) {
            if (entry->getId()  == entryId) {
                    entry->setAsRead();
                    feed->decrementUnreadCount();
                    _numberOfNewEntries--;
                    break;
            }
        }
    }
}

void Database::addFeedLabel(QString feedId, QString labelName) {
    foreach (Feed* feed, _storage) {
        if (feed->getId() == feedId) {
            feed->setLabel(labelName);
            break;
        }
    }
}

void Database::removeFeedLabel(QString feedId, QString labelName) {
    foreach (Feed* feed, _storage) {
        if (feed->getId() == feedId) {
            feed->removeLabel(labelName);
            break;
        }
    }
}

/*!
\brief
*/
int Database::getUnreadCountInFeed(QString feedId) {
    foreach (Feed* feed, _storage) {
        if (feed->getId() == feedId) {
            return feed->getUnreadCount();
        }
    }
    return 0;
}

/*!
\brief
*/
QList<Feed*> Database::getFeedsInLabel(QString label) {

    QList<Feed*> feedsList;

    foreach (Feed* feed, _storage) {
        QList<QString> tempLabelList = feed->getLabel();
        foreach (QString tempLabel, tempLabelList) {
            if (label == tempLabel) {
                feedsList.append(feed);
            }
        }
    }
    return feedsList;
}

/*!
\brief
*/
void Database::setNumberOfNewEntries(QString feedId, int numberOfNewEntries) {
    foreach (Feed* feed, _storage) {
        if (feed->getTitle() == feedId) {
            feed->setUnreadCount(numberOfNewEntries);
            break;
        }
    }
}

QList<QString> Database::getLabelsList() {

    QList<QString> labelList;

    foreach (Feed* feed, _storage) {
        QList<QString> templabelList = feed->getLabel();
        foreach (QString label, templabelList) {
            if (label != "" && !labelList.contains(label)) {
                labelList.append(label);
            }
        }
    }
    return labelList;
}

QString Database::getIdForFeed(QString name) {
    foreach (Feed* feed, _storage) {
        if (name == feed->getTitle()) {
            return feed->getId();
        }
    }
    return NULL;
}

QString Database::getEntryLink(QString id) {
    foreach (Feed* feed, _storage) {
        foreach (Entry* entry, feed->getEntriesList()) {
            if (entry->getId() == id) {
                return entry->getLink();
            }
        }
    }
    return NULL;
}

Feed* Database::getFeed(QString id) {
    foreach (Feed* feed, _storage)
        if (feed->getId() == id)
            return feed;

    return NULL;
}

Entry* Database::getEntry(QString id) {
    foreach (Feed* feed, _storage) {
        foreach (Entry* entry, feed->getEntriesList()) {
            if (entry->getId() == id) {
                return entry;
            }
        }
    }
    return NULL;
}

QString Database::getFeedIdForEntry(QString id) {
    foreach (Feed* feed, _storage) {
        foreach (Entry* entry, feed->getEntriesList()) {
            if (entry->getId() == id) {
                return feed->getId();
            }
        }
    }
    return NULL;
}

void Database::addFeeds(QByteArray source) {

    QXmlStreamReader sourceReader(source);

    QString id = "";
    QString title = "";
    QList<QString> label;
    QString firstitemmsec = "";

    bool inList = false;
    bool feedExists = false;

    //dokud nejsem na konci
    while (!sourceReader.atEnd()) {
        //prectu dalsi pocatecni element
        if (sourceReader.readNextStartElement()) {
            //pokud jsem v seznamu zdroju
            if (inList) {
                //hledam tagy string a number
                if (sourceReader.name() == "string" || sourceReader.name() == "number") {
                    //nactu si atributy prislusneho tagu
                    QXmlStreamAttributes attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    //pokud jsem na polozce id zdroje, ulozim ji
                    if (attributes.value((QString("name"))) == "id") {
                        if (sourceReader.text().toString().left(4) == "feed") {
                            id = sourceReader.text().toString().mid(5);
                            qDebug() << id;
                        }
                    }
                    //pokud jsem na polozce udavajici pocet neprectenych polozek
                    if (attributes.value((QString("name"))) == "title") {
                        title = sourceReader.text().toString();
                    }
                    if (attributes.value((QString("name"))) == "label") {
                        label.append(sourceReader.text().toString());
                    }
                    //pokud jsem na polozce udavajici cas nejnovejsi polozky
                    if (attributes.value((QString("name"))) == "firstitemmsec") {
                        firstitemmsec = sourceReader.text().toString();
                        //pokud mam vsechny potrebne informace o zdroji
                        if (!id.isEmpty() && !title.isEmpty() && !firstitemmsec.isEmpty()) {
                            //prochazim seznamem zdroju
                            foreach (Feed* feed, _storage)
                            {
                                //pokud jsem stejny zdroj nasel
                                if (feed->getId() == id) {
                                    feedExists = true;
                                }
                            }
                            if (!feedExists) {
                                //vytvorim novy zdroj
                                Feed* newFeed = new Feed(id, title, label, firstitemmsec);
                                id.clear();
                                title.clear();
                                label.clear();
                                //a vlozim do seznamu
                                _storage.append(newFeed);
                            }
                        }
                        feedExists = false;
                    }
                }
            }
            //pokud jsem v seznamu zdroju
            if (sourceReader.name() == "list") inList = true;
        }
    }
}

void Database::addUnreadFeeds(QByteArray source) {

    QXmlStreamReader sourceReader(source);

    QString id = "";
    int unreadCount = 0;
    QString newestItemTimestampUsec = "";

    bool inFeeds = false;
    bool inList = false;
    bool notInUser = false;

    _numberOfNewEntries = 0;

    //dokud nejsem na konci
    while (!sourceReader.atEnd()) {
        //prectu dalsi pocatecni element
        if (sourceReader.readNextStartElement()) {
            //pokud jsem v seznamu zdroju
            if (inList) {
                //hledam tagy string a number
                if (sourceReader.name() == "string" || sourceReader.name() == "number") {
                    //nactu si atributy prislusneho tagu
                    QXmlStreamAttributes attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    //pokud jsem opravdu u adresy zdroje (a ne napr. labelu)
                    if (sourceReader.text().toString().left(4) == "feed") inFeeds = true;
                    else inFeeds = false;
                    //pokud jsem na polozce id zdroje, ulozim ji
                    if (attributes.value((QString("name"))) == "id") {
                        if (inFeeds) {
                            id = sourceReader.text().toString().mid(5);
                            notInUser = true;
                        }
                    }
                    //pokud jsem na polozce udavajici pocet neprectenych polozek
                    if (attributes.value((QString("name"))) == "count") {
                        if (notInUser) {
                            unreadCount = sourceReader.text().toString().toInt();
                        }
                    }
                    //pokud jsem na polozce udavajici cas nejnovejsi polozky
                    if (attributes.value((QString("name"))) == "newestItemTimestampUsec") {
                        if (notInUser) {
                            newestItemTimestampUsec = sourceReader.text().toString();
                            //pokud mam vsechny potrebne informace o zdroji
                            if (!id.isEmpty() && !newestItemTimestampUsec.isEmpty()) {
                                //prochazim seznamem zdroju
                                foreach (Feed* feed, _storage) {
                                    //pokud jsem stejny zdroj nasel
                                    if (feed->getId() == id) {
                                        //nastavim novy pocet neprectenych polozek
                                        feed->setUnreadCount(unreadCount);
                                        feed->setNewestItemTimestamp(newestItemTimestampUsec.toInt());
                                        _numberOfNewEntries += unreadCount;
                                    }
                                }
                            }
                        }
                        notInUser = false;
                        inFeeds = false;
                    }
                }
            }
            //pokud jsem v seznamu zdroju
            if (sourceReader.name() == "list")
                inList = true;
        }
    }
}

void Database::addEntries(QByteArray source) {

    QXmlStreamReader sourceReader(source);

    QXmlStreamAttributes attributes;
    QString title = "";
    QString published = "";
    QString link = "";
    QString feedId = "";
    QString feedTitle = "";
    QString entryId = "";
    QString label = "";
    QString author = "";
    QString summary = "";

    bool inEvent = false;
    bool inSource = false;
    bool feedExists = false;
    bool entryExists = false;
    bool markedAsRead = false;

    Entry* newEntry;

    //dokud nejsem na konci XML
    while (!sourceReader.atEnd()) {
        //prectu dalsi pocatecni element
        if (sourceReader.readNextStartElement()) {
            //pokud jsem v casti tykajici se udalost
            if (inEvent) {
                //pokud jsem v casti popisujici zdroj
                if (sourceReader.name() == "title" && inSource) {
                    //ziskam atributy tagu
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    //ulozim nazev zdroje
                    feedTitle = sourceReader.text().toString();
                    //prochazim seznam zdroju, zda je tento zdroj jiz vlozen
                    foreach (Feed* feed, _storage) {
                        //pokud ano
                        if (feed->getId() == feedId) {
                            if (feed->getLabel().isEmpty()) feed->setLabel(label);
                            feedExists = true;
                        }
                    }
                    //pokud zdroj neni v seznamu
                    if (!feedExists) {
                        //vytvorim jej
                        Feed *newFeed = new Feed(feedId, feedTitle);
                        newFeed->setLabel(label);
                        //a vlozim do seznamu
                        _storage.append(newFeed);
                    }
                    //prochazim seznam zdroju
                    foreach (Feed* feed, _storage) {
                        //a hledam zdroj do ktereho patri dana polozka
                        if (feed->getId() == feedId) {
                            foreach (Entry* entry, feed->getEntriesList()) {
                                //pokud jsem nasel stejnou polozku nebudu ji vytvaret znovu
                                if (entry->getId() == entryId) entryExists = true;
                            }
                            if (!entryExists) {
                                //vytvorim novou polozku
                                newEntry = new Entry(title, published, link, entryId, author, summary, markedAsRead);
                                //vlozim polozku do zdroje
                                feed->entries.append(newEntry);
                                //pokud neni vyplnenen nazev zdroje vyplnim jej
                                if (feed->getTitle().isEmpty()) {
                                    feed->setTitle(feedTitle);
                                }
                            }
                        }
                    }
                    inSource = false;
                    inEvent = false;
                    feedExists = false;
                    entryExists = false;
                    markedAsRead = false;
                    label.clear();
                }
                //pokud jsem na tagu, ktery obsahuje id polozky
                if (sourceReader.name() == "id") {
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    entryId = sourceReader.text().toString();
                }
                //pokud jsem na tagu, ktery obsahuje kategorii
                if (sourceReader.name() == "category") {
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    //vyhledam si zda je polozka oznacena jako prectena
                    //tuto hodnotu pak vkladam pri vytvareni polozky
                    if (attributes.value(QString("label")) == "read") {
                        markedAsRead = true;

                    }
                    //pokud je zdroj prirazen do nejake slozky
                    if (attributes.value(QString("term")).toString().contains("label", Qt::CaseInsensitive)) {
                        label = attributes.value(QString("label")).toString();
                    }
                }
                //pokud jsem na tagu, ktery obsahuje nazev polozky
                else if (sourceReader.name() == "title") {
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    title = sourceReader.text().toString();
                }
                //pokud jsem na tagu, ktery obsahuje datum a cas publikovani
                else if (sourceReader.name() == "published")
                {
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    published = sourceReader.text().toString();
                }
                //pokud jsem na tagu, ktery obsahuje shrnuti
                else if (sourceReader.name() == "summary")
                {
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    summary = sourceReader.text().toString();
                }
                //pokud jsem na tagu, ktery obsahuje odkaz na internetove stranky, kde
                //je polozka publikovana
                else if (sourceReader.name() == "link") {
                    attributes = sourceReader.attributes();
                    if (attributes.value(QString("rel")) == "alternate") {
                        link = attributes.value(QString("href")).toString();
                    }
                }
                //pokud jsem na tagu, ktery obsahuje id zdroje
                else if (sourceReader.name() == "source") {
                    inSource = true;
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    feedId = attributes.value(QString("gr:stream-id")).toString().mid(5);
                }
                else if (sourceReader.name() == "author") {
                    inSource = true;
                    attributes = sourceReader.attributes();
                    sourceReader.readNext();
                    if (attributes.value(QString("gr:unknown-author")) == "true") author = trUtf8("Unknown author");
                    else {
                        sourceReader.readNext();
                        author = sourceReader.text().toString();
                    }
                }
            }
            //pokud jsem v casti oznacujici polozku
            if (sourceReader.name() == "entry") inEvent = true;
        }
    }
}
