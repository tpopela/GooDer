#include "GooDer.h"
#include "ui_GooDer.h"
#include "entry.h"
#include "addFeedDialog.h"
#include "removeFeedDialog.h"
#include "settingsDialog.h"
#include "labelNameDialog.h"

#include <QDebug>
#include <QtNetwork/QNetworkCookie>
#include <QFile>
#include <QtGui>
#include <QFont>
#include <QXmlStreamWriter>
#include <QShortcut>
#include <QSettings>

/*!
\brief Vychozi konstruktor
*/
GooDer::GooDer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/*!
\brief Vychozi destruktor
*/
GooDer::~GooDer()
{
    qDebug() << "Exiting";
    qDebug() << "";
    qDebug() << "";

    delete ui;
}

void GooDer::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        case QEvent::WindowStateChange:
            if (this->isMinimized()) {
                this->hide();
                this->setFlash(false);
                ui->browser->reload();
            }
            else {
                this->setFlash(false);
                ui->browser->reload();
            }
            break;
        case QEvent::ApplicationActivate:
            qDebug() << "Activate timer";
        case QEvent::ApplicationDeactivate:
            qDebug() << "Activate timer";
        default:
            break;
    }
}

void GooDer::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        this->show();
        break;
    case QSystemTrayIcon::MiddleClick:
        this->checkFeeds();
        break;
    default:
        ;
    }
}

/*!
\brief Inicializace prvku mimo konstruktor
*/
void GooDer::initialize() {

    _settings = new QSettings("GooDer", "GooDer");

    _googleReaderController = new GoogleReaderController();
    _crypt = new SimpleCrypt(Q_UINT64_C(0x0c2ac1a5cea3fe23));
//    SimpleCrypt crypt(Q_UINT64_C(0x0c2ac1a5cea3fe23));

    _timerRefreshFeeds = new QTimer(this);

    _checkFeedTime = 0;
    _currentLabel = 0;
    _commandHistoryCounter = -1;
    _currentFeed = 0;
    _feedAdded = false;
    _firstRun = true;
    _fetchingEntriesFromHistory = false;
    _fetchingEntriesFromHistoryActivated = true;

    //vytvori polozku v systemove liste
    createTray();
    //nastavi ikonu v systemove liste
    setIcon();
    //zobrazi polozku v systemove liste
    _trayIcon->show();
    //skryje listu s tlacitkem pro zobrazeni seznamu zdroju
    ui->frameShowFeedListButton->hide();
    //a pro hledani
    ui->frameSearch->hide();
    //nastavime pocet sloupcu v panelech se zdroji a polozkami
    ui->feedTreeWidget->setColumnCount(2);
    ui->feedTreeWidget->setRootIsDecorated(false);
    ui->entriesTreeWidget->setColumnCount(4);
    ui->entriesTreeWidget->setColumnWidth(4,0);
    ui->entriesTreeWidget->sortByColumn(2);
    //nastavime skryvani dlouhych slov
    ui->feedTreeWidget->setWordWrap(true);
    //nastavime jmena sloupcu v panelu s polozkami
    ui->entriesTreeWidget->headerItem()->setText(0, trUtf8("Entry name"));
    ui->entriesTreeWidget->headerItem()->setText(1, trUtf8("Feed"));
    ui->entriesTreeWidget->headerItem()->setText(2, trUtf8("Date"));
    ui->entriesTreeWidget->headerItem()->setText(3, trUtf8("Author"));
    ui->entriesTreeWidget->header()->show();

    //vytvorime kontextove menu v seznamu zdroju
    createFeedsContextMenu();
    createEntriesContextMenu();

    //napoji signaly
    this->connectSignals();

    connect(_googleReaderController, SIGNAL(signalConnected(bool)),
            this, SLOT(onlineState(bool)));

    //nastavi pismo a pocitadlo pro rozlisovani radku
    _fontNormal.setWeight(QFont::Normal);
    _fontBold.setWeight(QFont::Bold);
    _lineColor = QColor(220, 220, 220);
    _labelColor = QColor(187,201,255);

    this->setStatusbarWidgets();

    this->loadSettings();
}

void GooDer::setStatusbarWidgets() {

    _commandsLine = new QLineEdit();
    _loadProgress = new QLineEdit();

    QFontMetrics metrics(QApplication::font());

    QPalette palette = _commandsLine->palette();
    palette.setColor(QPalette::Base, Qt::transparent);

    _commandsLine->setPalette(palette);
    _loadProgress->setPalette(palette);

    _commandsLine->setFrame(false);
    _loadProgress->setFrame(false);

    _loadProgress->setReadOnly(true);

    _loadProgress->setFixedWidth(metrics.width("10000%"));
    _loadProgress->setAlignment(Qt::AlignRight);

    ui->statusbar->addPermanentWidget(_commandsLine);
    ui->statusbar->addPermanentWidget(_loadProgress);

    //nainstalujeme filtr pro odchytavani klaves nahoru a dolu v radce s prikazy
    _commandsLine->installEventFilter(this);
}

void GooDer::setFlash(bool flash) {

//    flashEnabled = flash;

    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
    if (flash) {
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);
        qDebug() << "flash is ON";
    }
    else {
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, false);
        qDebug() << "flash is OFF";
    }
}

void GooDer::loadSettings() {

    if (!_settings->value("username").toString().isNull()) {
        _shortcutNextEntry = (_settings->value("shortcutNextEntry", "Meta+.").toString());
        _shortcutPrevEntry = (_settings->value("shortcutPrevEntry", "Meta+,").toString());
        _shortcutNextFeed = (_settings->value("shortcutNextFeed", "Meta+'").toString());
        _shortcutPrevFeed = (_settings->value("shortcutPrevFeed", "Meta+;").toString());
        _shortcutNextLabel = (_settings->value("shortcutNextLabel", "Meta+]").toString());
        _shortcutPrevLabel = (_settings->value("shortcutPrevLabel", "Meta+[").toString());
        _shortcutToggleFeedPanel = (_settings->value("shortcutToggleFeedPanel", "Meta+\\").toString());
        _flashEnabled = _settings->value("flash", false).toBool();
        _showSummary = _settings->value("showSummary", false).toBool();
        _checkFeedTime = _settings->value("checkFeedTime", 300000).toInt();
        _autoHideFeedPanel = _settings->value("autoHideFeedPanel", true).toBool();
        _showAllFeeds = _settings->value("showAllFeeds", true).toBool();

        (_settings->value("toolbar", true).toBool()) ? ui->toolBar->show() : ui->toolBar->hide();
        (_settings->value("menuVisibility", true).toBool()) ? ui->menubar->show() : ui->menubar->hide();

        _timerRefreshFeeds->start(_checkFeedTime);

        setShortcuts();

        emit signalLogin();
    }
    else {
        emit this->showSettingsDialog();
    }
}

void GooDer::saveSettings() {

    _settings->setValue("username", _settings->value("username"));
    _settings->setValue("password", _settings->value("password"));
    _settings->setValue("checkFeedTime", _checkFeedTime);
    _settings->setValue("toolbar", ui->toolBar->isVisible());
    _settings->setValue("autoHideFeedPanel", _autoHideFeedPanel);
    _settings->setValue("menuVisibility", ui->menubar->isVisible());
    _settings->setValue("flash", _flashEnabled);
    _settings->setValue("showSummary", _showSummary);
    _settings->setValue("showAllFeeds", _showAllFeeds);
    _settings->setValue("shortcutNextEntry", _shortcutNextEntry);
    _settings->setValue("shortcutPrevEntry", _shortcutPrevEntry);
    _settings->setValue("shortcutNextFeed", _shortcutNextFeed);
    _settings->setValue("shortcutPrevFeed", _shortcutPrevFeed);
    _settings->setValue("shortcutNextLabel", _shortcutNextLabel);
    _settings->setValue("shortcutPrevLabel", _shortcutPrevLabel);
    _settings->setValue("shortcutToggleFeedPanel", _shortcutToggleFeedPanel);
}

/*!
\brief Ulozi veci potrebne k odesilani pozadavku
*/
void GooDer::loginSuccessfull(bool status) {
    if (status) {
        qDebug() << "Login was successfull -> checking feeds";
        emit checkFeeds();
    }
    else {
        qDebug() << "Login was unsuccessfull";
    }

}

/*!
\brief
*/
void GooDer::onlineState(bool online) {
    _online = online;
}

/*!
\brief Instalace filtru pro odchytaveni klaves nahoru a dolu v radku pro zadavani prikazu
*/
bool GooDer::eventFilter(QObject* obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationDeactivate)
    {
        qDebug() << "Deactivate timer";
        // Deactivate timer
    }
    if (event->type() == QEvent::ApplicationActivate)
    {
        qDebug() << "Activate timer";
        // Turn timer back on
    }

    if (obj == _commandsLine) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            //stisknuta klavesa nahoru
            if (keyEvent->key() == Qt::Key_Up) {
                if (!_commandHistory.isEmpty() && _commandHistoryCounter < _commandHistory.count() -1) {
                    _commandHistoryCounter++;
                    _commandsLine->setText(_commandHistory.at(_commandHistoryCounter));
                }
                return true;
            }
            //stisknuta klavesa dolu
            else if(keyEvent->key() == Qt::Key_Down) {
                if (!_commandHistory.isEmpty() && _commandHistoryCounter != -1) {
                    _commandHistoryCounter--;
                    if (_commandHistoryCounter != -1) {
                        _commandsLine->setText(_commandHistory.at(_commandHistoryCounter));
                    }
                    return true;
                }
            }
            else if (keyEvent->key() == Qt::Key_Escape) {

                ui->feedTreeWidget->setFocus();
            }
        }
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}

/*!
\brief Vytvori kontextove menu pro vytvoreni slozky
*/
void GooDer::createFeedsContextMenu() {
    //v seznamu zdroju
    ui->feedTreeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    _feedContextMenu = new QMenu(ui->feedTreeWidget);
    _actionCreateNewLabel = new QAction(trUtf8("Add label"),_feedContextMenu);
    ui->feedTreeWidget->addAction(_actionCreateNewLabel);
    _actionRemoveLabel = new QAction(trUtf8("Remove label"),_feedContextMenu);
    ui->feedTreeWidget->addAction(_actionRemoveLabel);
    _actionRemoveFeed = new QAction(trUtf8("Remove feed"),_feedContextMenu);
    ui->feedTreeWidget->addAction(_actionRemoveFeed);
}

/*!
\brief Vytvori kontextove menu pro vytvoreni slozky
*/
void GooDer::createEntriesContextMenu() {
    //v seznamu polozek
    ui->entriesTreeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    _entryContextMenu = new QMenu(ui->entriesTreeWidget);
    _actionOpenInExternBrowser = new QAction(trUtf8("Open in external browser"), _entryContextMenu);
    ui->entriesTreeWidget->addAction(_actionOpenInExternBrowser);
}

/*!
\brief Nastavi klavesove zkratky pouzivane v programu
*/
void GooDer::setShortcuts() {
    //odchytava dvojtecku a vola zadany slot
    new QShortcut(QKeySequence(Qt::Key_Colon), this, SLOT(showCommandLine()));
    //po stisknuti ZZ ukonci aplikaci
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Z, Qt::SHIFT + Qt::Key_Z), this, SLOT(close()));
    //po stisknuti CTRL + F nebo / se zobrazi pole pro vyhledavani
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this, SLOT(showSearchLine()));
    new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(showSearchLine()));
    //po stisknuti Escape se se pole skryje
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hideSearchCommandBar()));
    //po stisknuti CTRL + . prejde na dalsi polozku
    new QShortcut(QKeySequence::fromString(_shortcutNextEntry, QKeySequence::PortableText), this, SLOT(readNextEntry()));
    //po stisknuti CTRL + , prejde na predchozi polozku
    new QShortcut(QKeySequence::fromString(_shortcutPrevEntry, QKeySequence::PortableText), this, SLOT(readPreviousEntry()));
    //po stisknuti CTRL + ' prejde na dalsi zdroj
    new QShortcut(QKeySequence::fromString(_shortcutNextFeed, QKeySequence::PortableText), this, SLOT(readNextFeed()));
    //po stisknuti CTRL + ; prejdu na predchozi zdroj
    new QShortcut(QKeySequence::fromString(_shortcutPrevFeed, QKeySequence::PortableText), this, SLOT(readPreviousFeed()));
    //po stisknuti CTRL + [ prejde na predchozi stitek
    new QShortcut(QKeySequence::fromString(_shortcutPrevLabel, QKeySequence::PortableText), this, SLOT(readPreviousLabel()));
    //po stisknuti CTRL + ] prejde na dalsi stitek
    new QShortcut(QKeySequence::fromString(_shortcutNextLabel, QKeySequence::PortableText), this, SLOT(readNextLabel()));
    //po stisknuti CTRL + N zobrazi dialog pro pridani zroje
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(showAddFeedDialog()));
    //po stisknuti CTRL + R zobrazi dialog pro odebrani zdroje
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this, SLOT(showRemoveFeedDialog()));
    //po stisknuti CTRL + M oznaci polozky jako prectene
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this, SLOT(markAsRead()));
    //po stisknuti CTRL + P zobrazi nastaveni
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P), this, SLOT(showSettingsDialog()));
    //po stisknuti F5 aktualizuje zdroje
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(callCheckFeeds()));
    //po stisknuti F1 zobrazi napovedu
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
    //po stisknuti CTRL + \ zobrazi/skryje seznam zdroju
    new QShortcut(QKeySequence::fromString(_shortcutToggleFeedPanel, QKeySequence::PortableText), this, SLOT(showHideFeedList()));

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
}

/*!
\brief Rozpoznava prikazy a vola jednotlive ukony
*/
void GooDer::parseCommands() {

    //ulozim si pozadavek
    QString command = _commandsLine->text().mid(1);

    //vlozim jej do historie
    _commandHistory.append(_commandsLine->text());
    _commandsLine->clear();

    //pokud chci zkontrolovat zdroje
    if (command == "c" || command == "check") emit checkFeeds();
    //pokud chci zobrazit okno pro pridani noveho zdroje
    else if ((command == "a" || command == "add") && command.length() == 1) emit signalShowAddFeedDialog();
    //pokud chci pridat novy zdroj prikazem
    //pokud chceme pridal label aktualni polozce
    else if (command.left(7) == "a label") { emit signalAddLabel(command.mid(8)); return; }
    else if (command.left(9) == "add label") { emit signalAddLabel(command.mid(10)); return; }
    else if (command.left(2) == "a " && command.length() > 1) {
        QStringList feedData = command.split(" ");
        if (feedData.count() == 4) {
            emit signalAddFeed(feedData.at(1), feedData.at(2), feedData.at(3));
        }
        else showStatusBarMessage("Enter all required informations!");

    }
    //pokud chci odebrat zdroj
    else if (command == "rm" || command == "remove") emit signalShowRemoveFeedDialog();
    else if ((command == "remove this" || command == "rm this") && command.length() >= 7 ) {
        if (ui->feedTreeWidget->topLevelItemCount() > 0) {
            _googleReaderController->removeFeed(ui->feedTreeWidget->currentItem()->text(2));
        }
    }
    //pokud chceme odebrat stitek
    else if (command == "rm label" || command == "remove label") emit signalRemoveLabel();
    //pokud chci zobrazit nastaveni
    else if (command == "o" || command == "options") emit signalShowSettingsDialog();
    //pokud chceme aplikaci ukoncit
    else if (command == "wq" || command == "q" || command == "quit") qApp->quit();
    //pokud chceme aplikaci minimalizovat
    else if (command == "mi" || command == "minimize") this->showMinimized();
    //pokud chceme oznacit polozky jako prectene
    else if (command == "m" || command == "mark") this->markAsRead();
    //pokud chceme prejit na dalsi polozku
    else if (command == "n" || command == "next") emit signalReadNextEntry();
    //pokud chceme prejit na nasledujici zdroj
    else if (command == "nf") emit signalReadNextFeed();
    //pokud chceme zobrazit ci skyt menu
    else if (command == "tm") emit signalShowHideMenu();
    //pokud chceme prejit na nasledujici stitek
    else if (command == "nl") emit signalReadNextLabel();
    //pokud chceme prejit na prechozi polozku
    else if (command == "p" || command == "prev") emit signalReadPreviousEntry();
    //pokud chceme prejit na predchozi zdroj
    else if (command == "pf") emit signalReadPreviousFeed();
    //pokud chceme prejit na predchozi stitek
    else if (command == "pl") emit signalReadPreviousLabel();
    //zobrazi/skryje nastrojovou listu
    else if (command == "tt") emit signalShowHideToolbar();
    //pokud chci nastavit nejakou polozku
    else if (command.left(3) == "set") {
        //pokud chceme nastavit cas automaticke aktualizace zdroju
        if (command.mid(4,4) == "time") {
            _checkFeedTime = command.mid(9).toInt();
        }
        //pokud chceme nastavit viditelnost listy nastroju
        else if (command.mid(4,7) == "toolbar") {
            if (command.right(2) == "on") {
                ui->toolBar->show();
            }
            if (command.right(3) == "off") {
                ui->toolBar->hide();
            }
            saveSettings();
        }
        //pokud chceme nastavit viditelnost menu
        else if (command.mid(4,4) == "menu") {
            if (command.right(2) == "on") {
                ui->menubar->show();
            }
            if (command.right(3) == "off") {
                ui->menubar->hide();
            }
            saveSettings();
        }
        else if (command.mid(4,7) == "summary") {
            if (command.right(2) == "on") {
                _showSummary = true;
            }
            if (command.right(3) == "off") {
                _showSummary = false;
            }
            saveSettings();
        }
        else if (command.mid(4,5) == "flash") {
            if (command.right(2) == "on") {
                this->setFlash(true);
                ui->browser->reload();
                _flashEnabled = true;
                this->setStatusBarMessage("Flash is enabled");
            }
            else {
                this->setFlash(false);
                ui->browser->reload();
                _flashEnabled = false;
                this->setStatusBarMessage("Flash is disabled");
            }
            saveSettings();
        }
        else if (command.mid(4,7) == "showall") {
            if (command.right(2) == "on") {
                _showAllFeeds = true;
                ui->feedTreeWidget->topLevelItem(0)->setHidden(false);
            }
            else {
                _showAllFeeds = false;
                ui->feedTreeWidget->topLevelItem(0)->setHidden(true);
            }
            saveSettings();
        }
        //pokud chceme nastavit automaticke skryvani panelu se zdroji
        else if (command.mid(4,8) == "autohide") {
            if (command.right(2) == "on") {
                _autoHideFeedPanel = true;
            }
            if (command.right(3) == "off") {
                _autoHideFeedPanel = false;
                ui->frameShowFeedListButton->hide();
                ui->feedTreeWidget->show();
            }
            saveSettings();
        }
        else {
            showStatusBarMessage("Unknown command: " + command);
        }
    }
    else if (command.left(4) == "info") {
        //pokud chceme nastavit cas automaticke aktualizace zdroju
        if (command.mid(5,4) == "time") {
            _commandsLine->setText(command.mid(5,4) + " " + QString::number(_checkFeedTime/60000));
        }
        //pokud chceme nastavit viditelnost listy nastroju
        else if (command.mid(5,7) == "toolbar") {
            if (ui->toolBar->isHidden()) {
                _commandsLine->setText(command.mid(5,7) + " " + "off");
            }
            else {
                _commandsLine->setText(command.mid(5,7) + " " + "on");
            }
        }
        //pokud chceme nastavit viditelnost menu
        else if (command.mid(5,4) == "menu") {
            if (ui->menubar->isHidden()) {
                _commandsLine->setText(command.mid(5,4) + " " + "off");
            }
            else {
                _commandsLine->setText(command.mid(5,4) + " " + "on");
            }
        }
        else if (command.mid(5,5) == "flash") {
            if (_flashEnabled) {
                _commandsLine->setText(command.mid(5,5) + " " + "on");
            }
            else {
                _commandsLine->setText(command.mid(5,5) + " " + "off");
            }
        }
        //pokud chceme nastavit heslo k Google Reader
        else if (command.mid(5,8) == "password") {
            _commandsLine->setText(_settings->value("password").toString());
        }
        //pokud chceme nastavit uzivatelske jmeno k Google Reader
        else if (command.mid(5,8) == "username") {
            _commandsLine->setText(_settings->value("username").toString());
        }
        //pokud chceme nastavit automaticke skryvani panelu se zdroji
        else if (command.mid(5,8) == "autohide") {
            if (_autoHideFeedPanel) {
                _commandsLine->setText(command.mid(5,8) + " " + "on");
            }
            else {
                _commandsLine->setText(command.mid(5,8) + " " + "off");
            }
        }
        else {
            showStatusBarMessage("Unknown command: " + command);
        }
    }
    else if (command == "") {
        ui->entriesTreeWidget->setFocus();
    }
    else {
        showStatusBarMessage("Unknown command: " + command);
    }
    ui->entriesTreeWidget->setFocus();
    command.clear();
}

void GooDer::newFeedAdded() {
    _feedAdded = true;
    this->callCheckFeeds();
}

/*!
\brief Zavola metodu pro ziskani seznamu neprectenych polozek
*/
void GooDer::callCheckFeeds() {

    if (_online) {
        qDebug() << "fetching feeds" << " in GooDer::callCheckFeeds";
        _googleReaderController->getFeeds();
    }
    else {
        showStatusBarMessage("Connect to internet!");
        emit signalLogin();
    }
}

/*!
\brief Porovna dve ruzna data publikovani
*/
bool isLess(Entry* entry1, Entry* entry2) {

    if (entry1->getPublishedDate() <= entry2->getPublishedDate()) return false;
    else return true;

}

void GooDer::setEntriesWidget() {

    checkEntriesFeedsNumbers();

    int width = ui->entriesTreeWidget->header()->width();

    ui->entriesTreeWidget->header()->setResizeMode(0,QHeaderView::Interactive);
    ui->entriesTreeWidget->header()->setResizeMode(1,QHeaderView::Interactive);
    ui->entriesTreeWidget->header()->setResizeMode(2,QHeaderView::Interactive);
    ui->entriesTreeWidget->header()->setResizeMode(3,QHeaderView::Interactive);

    ui->entriesTreeWidget->setWordWrap(true);

    ui->entriesTreeWidget->setColumnWidth(0, width/3);
    int newWidth = width - width/3;

    ui->entriesTreeWidget->setColumnWidth(1,newWidth/3);
    ui->entriesTreeWidget->setColumnWidth(2,newWidth/3);
    ui->entriesTreeWidget->setColumnWidth(3,newWidth/3);
}

void GooDer::s_refreshAfterFeedDeleted() {
    this->readNextEntry();
    refreshFeedWidget();
}

void GooDer::s_refreshFeedWidget() {
    refreshFeedWidget();
}

void GooDer::refreshFeedWidget() {

    ui->feedTreeWidget->clear();

    bool notFound = true;

    QTreeWidgetItem* treeWidgetItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(trUtf8("All feeds")));
    ui->feedTreeWidget->addTopLevelItem(treeWidgetItem);

    foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
        //a pridavam je do leveho sloupce se zdroji
        QTreeWidgetItem *newEntry = new QTreeWidgetItem;
        newEntry->setText(0, feed->getTitle());
        newEntry->setText(2, feed->getId());
        treeWidgetItem->addChild(newEntry);

        //pridani noveho prvku s nazvem stitku
        if (!feed->getLabel().isEmpty()) {
            //seznam vsech stitku prirazenych zdroji
            QList<QString> labelList = feed->getLabel();
            //prochazim jej
            foreach (QString label, labelList) {
                if (!label.isEmpty()) {
                    //a pokousim se najit zda je jiz v levem sloupci se zdroji uveden
                    QList<QTreeWidgetItem*> itemsFound = ui->feedTreeWidget->findItems(label, Qt::MatchExactly, 0);
                    if (itemsFound.isEmpty()) {
                        ui->feedTreeWidget->addTopLevelItem(new QTreeWidgetItem((QTreeWidget*)0, QStringList(label)));
                    }
                    QList<QTreeWidgetItem*> labels = ui->feedTreeWidget->findItems(label, Qt::MatchExactly, 0);

                    for (int i = 0; i < labels.at(0)->childCount(); i++) {
                        if (labels.at(0)->child(i)->text(0) == feed->getTitle()) {
                            notFound = false;
                        }
                    }
                    if (notFound) {
                        newEntry = new QTreeWidgetItem;
                        newEntry->setText(0, feed->getTitle());
                        newEntry->setText(2, feed->getId());
                        labels.at(0)->addChild(newEntry);
                    }
                    notFound = true;
               }
            }
        }
        _feedAdded = false;
    }

    differentiateFeedsLines();
    checkEntriesFeedsNumbers();

    //rozbali vsechny polozky
    ui->feedTreeWidget->expandAll();
    //prizpusobim velikost bunek obsahu
    ui->feedTreeWidget->resizeColumnToContents(0);
    ui->feedTreeWidget->resizeColumnToContents(1);
    ui->feedTreeWidget->setWordWrap(true);
    setFeedListWidth();

    ui->entriesTreeWidget->setRootIsDecorated(false);
}

/*!
\brief Po spusteni programu, pokud neni zvolen zadny zdroj, zobrazim vsechy polozky serazene podle data
*/
void GooDer::getEntriesReady() {

    if (_fetchingEntriesFromHistory)
    {
        getEntriesFromFeed(_fetchingEntriesFromHistoryActivated);
        return;
    }

    this->showNotification();

    ui->statusbar->clearMessage();

    if (_firstRun || _feedAdded) {
        refreshFeedWidget();
    }

    if (!_firstRun) {
/////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            if (feed->getTitle() == ui->feedTreeWidget->currentItem()->text(0) && feed->getUnreadCount() > 0) {
                ui->entriesTreeWidget->clear();
                foreach (Entry* entry, feed->getEntriesList()) {
                    QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                    newEntry->setText(0, entry->getTitle());
                    newEntry->setText(1, feed->getTitle());
                    newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                    newEntry->setText(3, entry->getAuthor());
                    newEntry->setText(4, entry->getId());

                    if (!entry->isRead()) {
                        newEntry->setFont(0, _fontBold);
                        newEntry->setFont(1, _fontBold);
                        newEntry->setFont(2, _fontBold);
                        newEntry->setFont(3, _fontBold);
                    }
                    ui->entriesTreeWidget->addTopLevelItem(newEntry);
                }
                break;
            }
        }

        checkEntriesFeedsNumbers();
        differentiateEntriesLines();
    }

    if (_firstRun) {

        //zjistit jestli je nova polozka v aktualne zvolenym zdroji.. podle toho prepisovat ve widgetu
        ui->entriesTreeWidget->clear();

        //a vypisi jej do praveho sloupce s polozkami
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            foreach (Entry* entry, feed->getEntriesList()) {
                QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                newEntry->setText(0, entry->getTitle());
                newEntry->setText(1, feed->getTitle());
                newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                newEntry->setText(3, entry->getAuthor());
                newEntry->setText(4, entry->getId());

                if (!entry->isRead()) {
                    newEntry->setFont(0, _fontBold);
                    newEntry->setFont(1, _fontBold);
                    newEntry->setFont(2, _fontBold);
                    newEntry->setFont(3, _fontBold);
                }
                ui->entriesTreeWidget->addTopLevelItem(newEntry);
            }
        }

        differentiateEntriesLines();

        _firstRun = false;
    }
    if (!_showAllFeeds) ui->feedTreeWidget->topLevelItem(0)->setHidden(true);
}

/*!
\brief Zobrazi pole pro zadavani prikazu
*/
void GooDer::showCommandLine() {

    this->_commandsLine->setFocus();
    _commandsLine->setText(":");

    connect(_commandsLine, SIGNAL(returnPressed()),
        this, SLOT(parseCommands()));
}

/*!
\brief Zobrazi radek pro hledani
*/
void GooDer::showSearchLine() {
    if (ui->frameSearch->isHidden()) {
        ui->frameSearch->show();
        ui->searchKeyWord->setFocus();
    }
}

/*!
\brief Zobrazi dialog s nastavenim
*/
void GooDer::showSettingsDialog() {
    _settingsDialogInstance = new SettingsDialog(this);

    connect(_settingsDialogInstance, SIGNAL(settingsData()),
        this, SLOT(showSettingsDialogSuccess()));

    _settingsDialogInstance->exec();
}

void GooDer::showSettingsDialogSuccess() {
    loadSettings();
}

/*!
\brief Vola funkci pro prihlaseni programu
*/
void GooDer::login() {
    _googleReaderController->login(_settings->value("username").toString(),
                                  _crypt->decryptToString(_settings->value("password").toString()));
}

/*!
\brief Nastavi ikonu programu a ikonu v systemove liste
*/
void GooDer::setIcon() {
    QIcon icon = QIcon(":/icons/icons/GooDer.svgz");
    _trayIcon->setIcon(icon);
    GooDer::setWindowIcon(icon);
}

void GooDer::trayMarkAllAsRead() {
    this->markAllAsRead();
}

/*!
\brief Vytvori ikonu v systemove liste
*/
void GooDer::createTray() {
    _trayIcon = new QSystemTrayIcon(this);

    _trayMenu = new QMenu(this);
    QAction* trayCheckFeeds = new QAction(trUtf8("Check feeds"), this);
    connect(trayCheckFeeds, SIGNAL(triggered()),
            this, SLOT(callCheckFeeds()));
    _trayMenu->addAction(trayCheckFeeds);

    _trayMenu->addSeparator();

    QAction* trayMarkAllAsRead = new QAction(trUtf8("Mark all as read"), this);
    connect(trayMarkAllAsRead, SIGNAL(triggered()),
            this, SLOT(trayMarkAllAsRead()));

    _trayMenu->addAction(trayMarkAllAsRead);

    _trayMenu->addSeparator();

    QAction* trayExit = new QAction(trUtf8("Quit"), this);
    connect(trayExit, SIGNAL(triggered()),
            this, SLOT(close()));
    _trayMenu->addAction(trayExit);

    _trayIcon->setContextMenu(_trayMenu);
    connect(_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
}

/*!
\brief Zobrazi dialog O aplikaci
*/
void GooDer::showAbout() {
    QMessageBox::information(0, trUtf8("About GooDer"), trUtf8("Based on my bachelors thesis. Author: Tomáš Popela, tomas.popela@gmail.com"));
}

/*!
\brief Zobrazi dialog O aplikaci
*/
void GooDer::showHelp() {
    ui->browser->load(QUrl(":/help/help.html"));
}

void GooDer::connectSignalsUI() {

    //pokud chceme zobrazit napovedu
    connect(ui->actionHelp, SIGNAL(triggered()),
            this, SLOT(showHelp()));
    //pokud chceme zobrazit okno O aplikaci
    connect(ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAbout()));
    //pokud je v menu zvolena moznost pridat zdroj
    connect(ui->actionAddFeed, SIGNAL(triggered()),
            this, SLOT(showAddFeedDialog()));
    //pokud je zvolena moznost ukoncit program
    connect(ui->actionQuit, SIGNAL(triggered()),
            this, SLOT(close()));
    //pokud je zvolena moznost aktualizovat zdroje
    connect(ui->actionCheckFeeds, SIGNAL(triggered()),
            this, SLOT(callCheckFeeds()));
    //pokud je zvolena moznost oznacit vsechny polozky jako prectene
    connect(ui->actionMarkedAllAsRead, SIGNAL(triggered()),
            this, SLOT(markAsRead()));
    //pokud je zvolena moznost odstranit zdroj
    connect(ui->actionRemoveFeed, SIGNAL(triggered()),
            this, SLOT(showRemoveFeedDialog()));
    //pokud je zvolena moznost zobrazit nastaven*/i
    connect(ui->actionSettings, SIGNAL(triggered()),
            this, SLOT(showSettingsDialog()));
    //pokud je stisknuto tlacitko pro zobrazeni seznamu zdroju
    connect(ui->showFeedList, SIGNAL(clicked()),
            this, SLOT(showFeedTreeWidget()));
    //zobrazi seznam polozek po jednom kliknuti (Mac, Win)
    connect(ui->feedTreeWidget, SIGNAL(clicked(QModelIndex)),
            this, SLOT(showEntriesOnSelectedFeed(QModelIndex)));
    //zobrazi polozku po jednom kliknuti (Mac, Win)
    connect(ui->entriesTreeWidget, SIGNAL(clicked(QModelIndex)),
            this, SLOT(showSelectedEntry(QModelIndex)));
    //pokud je zadan pozadavek na vyhledavani
    connect(ui->searchKeyWord, SIGNAL(returnPressed()),
            this, SLOT(searchEntry()));
    connect(_trayIcon, SIGNAL(messageClicked()),
            this, SLOT(show()));
    connect(_trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(ui->browser, SIGNAL(loadProgress(int)),
            this, SLOT(showLoadProgress(int)));

    connect(ui->entriesTreeWidget->header(), SIGNAL(sectionClicked(int)),
            this, SLOT(changeSortingUIChanges(int)));
}

void GooDer::connectSignalsControl() {

    //zobrazi zpravu v informacni liste
    connect(this, SIGNAL(setStatusBarMessage(QString)),
            this, SLOT(showStatusBarMessage(QString)));
    //zkontroluje zdroje
    connect(this, SIGNAL(checkFeeds()),
            this, SLOT(callCheckFeeds()));
    //volani pro zobrazeni polozky v externim prohlizeci
    connect(this->_actionOpenInExternBrowser, SIGNAL(triggered()),
            this, SLOT(openEntryInExternalBrowser()));
    //prechod na nasledujici polozku
    connect(this, SIGNAL(signalReadNextEntry(QModelIndex)),
            this, SLOT(showSelectedEntry(QModelIndex)));
    //prechod na predchozi polozku
    connect(this, SIGNAL(signalReadPreviousEntry(QModelIndex)),
            this, SLOT(showSelectedEntry(QModelIndex)));
    //prechod na nasledujici polozku
    connect(this, SIGNAL(signalReadNextEntry()),
            this, SLOT(readNextEntry()));
    //prechod na predchozi polozku
    connect(this, SIGNAL(signalReadPreviousEntry()),
            this, SLOT(readPreviousEntry()));
    //prechod na nasledujici zdroj
    connect(this, SIGNAL(signalReadNextFeed(QModelIndex)),
            this, SLOT(showEntriesOnSelectedFeed(QModelIndex)));
    //prechod na predchozi zdroj
    connect(this, SIGNAL(signalReadPreviousFeed(QModelIndex)),
            this, SLOT(showEntriesOnSelectedFeed(QModelIndex)));
    //prechod na nasledujici zdroj
    connect(this, SIGNAL(signalReadNextFeed()),
            this, SLOT(readNextFeed()));
    //prechod na predchozi zdroj
    connect(this, SIGNAL(signalReadPreviousFeed()),
            this, SLOT(readPreviousFeed()));
    //prechod na nasledujici stitek
    connect(this, SIGNAL(signalReadNextLabel()),
            this, SLOT(readNextLabel()));
    //prechod na predchozi stitek
    connect(this, SIGNAL(signalReadPreviousLabel()),
            this, SLOT(readPreviousLabel()));
    //zobrazi/skryje nastrojovou listu
    connect(this, SIGNAL(signalShowHideToolbar()),
            this, SLOT(showHideToolbar()));
    //pokud je stisknuto tlacitko pro zobrazeni nastaveni
    connect(this, SIGNAL(signalShowSettingsDialog()),
            this, SLOT(showSettingsDialog()));
    //volani prihlaseni
    connect(this, SIGNAL(signalLogin()),
            this, SLOT(login()));
    //pokud je zvolena moznost vytvorit novou slozku
    connect(this->_actionCreateNewLabel, SIGNAL(triggered()),
            this, SLOT(showCreateNewLabelDialog()));
    //pokud je zvolena moznost odebrani stitku
    connect(this->_actionRemoveLabel, SIGNAL(triggered()),
            this, SLOT(removeLabel()));
    //pokud je zvolena moznost odebrani stitku
    connect(this->_actionRemoveFeed, SIGNAL(triggered()),
            this, SLOT(removeFeed()));
    //zobrazi dialog pro pridani noveho zdroje
    connect(this, SIGNAL(signalShowAddFeedDialog()),
            this, SLOT(showAddFeedDialog()));
    //zobrazi dialog pro odebrani zdroje
    connect(this, SIGNAL(signalShowRemoveFeedDialog()),
            this, SLOT(showRemoveFeedDialog()));
    //pokud chci pridat novy zdroj
    connect(this, SIGNAL(signalAddFeed(QString, QString, QString)),
            this, SLOT(addNewFeed(QString,QString, QString)));
    //pokud chceme zobrazit/skryt menu
    connect(this, SIGNAL(signalShowHideMenu()),
            this, SLOT(showHideMenu()));
    //pokud chceme odebrat stitek
    connect(this, SIGNAL(signalRemoveLabel()),
            this, SLOT(removeLabel()));
    //pokud chceme pridat stitek
    connect(this, SIGNAL(signalAddLabel(QString)),
            this, SLOT(createNewLabel(QString)));
}

void GooDer::connectSignalsAccount() {

    connect(_googleReaderController, SIGNAL(signalLoginStatus(bool)),
            this, SLOT(loginSuccessfull(bool)));
}

void GooDer::connectSignalsNetwork() {

    connect(_googleReaderController, SIGNAL(signalStatusLogin(bool)),
            this, SLOT(statusLogin(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusMarkEntryAsRead(bool)),
            this, SLOT(statusMarkEntryAsRead(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusMarkFeedAsRead(bool)),
            this, SLOT(statusMarkFeedAsRead(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusAddFeedLabel(bool)),
            this, SLOT(statusAddFeedLabel(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusRemoveFeedLabel(bool)),
            this, SLOT(statusRemoveFeedLabel(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusAddFeed(bool)),
            this, SLOT(statusAddFeedLabel(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusRemoveFeed(bool)),
            this, SLOT(statusRemoveFeedLabel(bool)));
    connect(_googleReaderController, SIGNAL(signalStatusGetEntries(bool)),
            this, SLOT(statusGetEntries(bool)));
    //nastavi hledani novych polozek po uplynuti casovace
    connect(_timerRefreshFeeds, SIGNAL(timeout()),
            this, SLOT(callCheckFeeds()));
}

/*!
\brief Spoji signaly se sloty
*/
void GooDer::connectSignals() {
    connectSignalsUI();
    connectSignalsNetwork();
    connectSignalsAccount();
    connectSignalsControl();
}

void GooDer::checkUInumbers() {
    checkEntriesFeedsNumbers();
}

/*!
\brief Oznaci vsechny polozky jako prectene
*/
void GooDer::markAllAsRead() {

    _googleReaderController->markAllAsRead();

    //nastavim vzhled polozek ve sloupci se zdroji jako prectene polozky
    markAllUIFeedsListAsRead();
    //v seznamu vsech polozek oznacim polozky jako prectene
    markAllUIEntriesListAsRead();
    checkEntriesFeedsNumbers();
    setIcon();
}

/*!
\brief Oznaci vsechny polozky z jednoho stitku(slozky) jako prectene
*/
void GooDer::markLabelAsRead() {

    _googleReaderController->markLabelAsRead(ui->feedTreeWidget->currentItem()->text(0));
    //v seznamu vsech polozek oznacim polozky jako prectene
    markAllUIEntriesListAsRead();
    checkEntriesFeedsNumbers();
}

/*!
\brief Oznaci polozky jednoho zdroje jako prectene
*/
void GooDer::markFeedAsRead() {
    _googleReaderController->markFeedAsRead(ui->feedTreeWidget->currentItem()->text(2));
    //nastavim vzhled polozek ve sloupci se zdroji jako prectene polozky
    markAllUIEntriesListAsRead();
    //zkontroluji pocet novych polozek
    checkEntriesFeedsNumbers();
}

/*!
\brief Podle vybraneho prvku ve sloupci se zdroji vola funkci pro oznaceni prvku jako prectene
*/
void GooDer::markAsRead() {

    //prochazim panel s polozkami
    if (ui->entriesTreeWidget->topLevelItemCount() > 0) {
        if (ui->feedTreeWidget->currentIndex().row() != -1) {
            //pokud je zvolena polozka Vsechny zdroje
            if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(0)->text(0)) {
                markAllAsRead();
            }
            //po kliknuti na stitek
            if (_googleReaderController->getUnreadCountInLabel(ui->feedTreeWidget->currentItem()->text(0)) != -1)
                markLabelAsRead();
            else
                markFeedAsRead();
        }
        else {
            markAllAsRead();
        }

        if (_googleReaderController->getTotalUnreadCount() == 0)
            setIcon();
        //        showStatusBarMessage("Položky úspěšně označeny jako přečtené");
    }
}

/*!
\brief Projde seznam zdroju v levem sloupci a upravi pocet neprectenych polozek
*/
void GooDer::checkEntriesFeedsNumbers() {

    int sumUnreadEntriesLabel = 0;

    //prochazim seznam polozek a zdroju
    for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
        for (int j  = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
            int unreadCount = _googleReaderController->getUnreadCountInFeed(ui->feedTreeWidget->topLevelItem(i)->child(j)->text(2));
            if (unreadCount > 0) {
                //oznacim zdroj jako neprecteny
                markUIFeedAsUnread(i,j,unreadCount);
            }
            else {
                markUIFeedAsRead(i,j);
            }
        }
    }

    sumUnreadEntriesLabel = 0;

    //labels
    for (int k = 0; k < ui->feedTreeWidget->topLevelItemCount(); k++) {
        QString labelName = ui->feedTreeWidget->topLevelItem(k)->text(0);
        sumUnreadEntriesLabel = _googleReaderController->getUnreadCountInLabel(labelName);

        //not summary
        if (k != 0) {
            ui->feedTreeWidget->topLevelItem(k)->setText(1, QString::number(sumUnreadEntriesLabel));
            if (sumUnreadEntriesLabel > 0) {
                ui->feedTreeWidget->topLevelItem(k)->setFont(0,_fontBold);
                ui->feedTreeWidget->topLevelItem(k)->setFont(1,_fontBold);
            }
            else {
                ui->feedTreeWidget->topLevelItem(k)->setFont(0,_fontNormal);
                ui->feedTreeWidget->topLevelItem(k)->setFont(1,_fontNormal);
            }
            sumUnreadEntriesLabel = 0;
        }
    }

    //nastavi pocet vsech neprectenych polozek
    int numberOfNewEntries = _googleReaderController->getTotalUnreadCount();
    ui->feedTreeWidget->topLevelItem(0)->setText(1, QString::number(numberOfNewEntries));
    if (numberOfNewEntries > 0) {
        ui->feedTreeWidget->topLevelItem(0)->setFont(0,_fontBold);
        ui->feedTreeWidget->topLevelItem(0)->setFont(1,_fontBold);
    }
    else {
        ui->feedTreeWidget->topLevelItem(0)->setFont(0,_fontNormal);
        ui->feedTreeWidget->topLevelItem(0)->setFont(1,_fontNormal);
        setIcon();
    }
}

/*!
\brief Zobrazi dialog pro zadani jmena stitku
*/
void GooDer::showCreateNewLabelDialog() {

    QList<QString> labelList;
    labelList.append(trUtf8("No label"));
    labelList.append(trUtf8("Add new label"));

    labelList.append(_googleReaderController->getLabelsList());

    _labelNameDialogInstance = new LabelNameDialog(labelList, this);
    connect (_labelNameDialogInstance, SIGNAL(signalLabelName(QString)),
            this, SLOT(createNewLabel(QString)));
    _labelNameDialogInstance->exec();
}

/*!
\brief Vytvori novy stitek
*/
void GooDer::createNewLabel(QString labelName) {

    QString id = "";
    bool notLabel = true;

    id = _googleReaderController->getIdForFeed(ui->feedTreeWidget->currentItem()->text(0));

    for (int j = 0; j < ui->feedTreeWidget->topLevelItemCount(); j++) {
        if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(j)->text(0))
            notLabel = false;
    }

    if (notLabel)
        _googleReaderController->addFeedLabel(id, labelName);
    else
        showStatusBarMessage("Can't create label on label!");
}

/*!
\brief Odstrani stitek z aktualne zvoleneho zdroje
*/
void GooDer::removeLabel() {
    if (ui->feedTreeWidget->currentItem()->parent()->text(0) != ui->feedTreeWidget->topLevelItem(0)->text(0)) {
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            if (ui->feedTreeWidget->currentItem()->text(0) == feed->getTitle()) {
                _googleReaderController->removeFeedLabel(feed->getId(), ui->feedTreeWidget->currentItem()->parent()->text(0));
                break;
            }
        }
    }
}

/*!
\brief Ziska seznam polozek z historie
*/
void GooDer::getEntriesFromFeed(bool moveToNextEntry) {
    _fetchingEntriesFromHistory = false;
    if (ui->feedTreeWidget->currentIndex().row() != -1) {
        //pokud nejsem na polozce Vsechny zdroje
        if (ui->feedTreeWidget->currentItem()->text(0) != ui->feedTreeWidget->topLevelItem(0)->text(0)){

            //predchozi obsah vymazu (pokud bych toto neudelal, nove polozky by se pridavali za aktualni
            ui->entriesTreeWidget->clear();

            //do panelu s polozkami vypisu vsechny polozky ze zdroje i s nove ziskanymi
            foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
                if (feed->getTitle() == ui->feedTreeWidget->currentItem()->text(0)) {
                    foreach (Entry* entry, feed->getEntriesList()) {

                        QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                        newEntry->setText(0, entry->getTitle());
                        newEntry->setText(1, feed->getTitle());
                        newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                        newEntry->setText(3, entry->getAuthor());
                        newEntry->setText(4, entry->getId());
                        if (!entry->isRead()) {
                            newEntry->setFont(0, _fontBold);
                            newEntry->setFont(1, _fontBold);
                            newEntry->setFont(2, _fontBold);
                            newEntry->setFont(3, _fontBold);
                        }
                        ui->entriesTreeWidget->addTopLevelItem(newEntry);
                    }
                }
            }
            differentiateEntriesLines();
            //pokud mam prejit na nasledujici polozku
            if (moveToNextEntry) {
                if (_lastEntriesCount != ui->entriesTreeWidget->topLevelItemCount()) {
                    ui->entriesTreeWidget->setCurrentItem(ui->entriesTreeWidget->topLevelItem(ui->entriesTreeWidget->topLevelItemCount()-5));
                    emit signalReadNextEntry(ui->entriesTreeWidget->currentIndex());
                }
                else {
                    ui->entriesTreeWidget->setCurrentItem(ui->entriesTreeWidget->topLevelItem(ui->entriesTreeWidget->topLevelItemCount()-1));
                }
            }
            //pokud mam zustat na aktualni polozce
            else {
                if (_lastEntriesCount != ui->entriesTreeWidget->topLevelItemCount()) {
                    ui->entriesTreeWidget->setCurrentItem(ui->entriesTreeWidget->topLevelItem(ui->entriesTreeWidget->topLevelItemCount()-6));
                }
                else {
                    ui->entriesTreeWidget->setCurrentItem(ui->entriesTreeWidget->topLevelItem(ui->entriesTreeWidget->topLevelItemCount()-1));
                }
            }
        }
    }
}

/*!
\brief Nastavi sirku seznamu se zdroji
*/
void GooDer::setFeedListWidth() {

    int fontWidth = 0;
    int numberWidth = 0;
    int feedListWidth = 0;

    for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
        for (int j = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
            if (fontWidth < ui->feedTreeWidget->fontMetrics().width(ui->feedTreeWidget->topLevelItem(i)->child(j)->text(0)))
                fontWidth = ui->feedTreeWidget->fontMetrics().width(ui->feedTreeWidget->topLevelItem(i)->child(j)->text(0));
            if (numberWidth < ui->feedTreeWidget->fontMetrics().width(ui->feedTreeWidget->topLevelItem(i)->child(j)->text(1)))
                numberWidth = ui->feedTreeWidget->fontMetrics().width(ui->feedTreeWidget->topLevelItem(i)->child(j)->text(1));
        }
    }

    if (fontWidth > 160) {
        feedListWidth = 185 + numberWidth + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 60;
        ui->feedTreeWidget->setColumnWidth(0, 250);
    }
    else {
        feedListWidth = fontWidth + numberWidth + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 60;
    }

    ui->feedTreeWidget->setFixedWidth(feedListWidth - 20);
}

/*!
\brief Po zvoleni zdroje ci stitku zobrazi polozky, ktere mu nalezi
*/
void GooDer::showEntriesOnSelectedFeed(QModelIndex index) {
    Q_UNUSED(index);

    ui->entriesTreeWidget->scrollToTop();
    //predchozi obsah vymazu (pokud bych toto neudelal, nove polozky by se pridavali za aktualni
    ui->entriesTreeWidget->clear();

    int sum = 0;

    if (ui->feedTreeWidget->topLevelItemCount() <= 0)
        return;

    //pokud je zvolena polozka Vsechny zdroje
    if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(0)->text(0)) {
        //vlozim do ni vsechny odebirane zdroje
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            foreach (Entry* entry, feed->getEntriesList()) {
                QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                //nastavim informace, o polozce
                newEntry->setText(0, entry->getTitle());
                newEntry->setText(1, feed->getTitle());
                newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                newEntry->setText(3, entry->getAuthor());
                newEntry->setText(4, entry->getId());
                if (!entry->isRead()) {
                    newEntry->setFont(0, _fontBold);
                    newEntry->setFont(1, _fontBold);
                    newEntry->setFont(2, _fontBold);
                    newEntry->setFont(3, _fontBold);
                }
                ui->entriesTreeWidget->addTopLevelItem(newEntry);
            }
        }
    }
    //po kliknuti na label
    else if (_googleReaderController->getUnreadCountInLabel(ui->feedTreeWidget->currentItem()->text(0)) != -1) {
        for (int m = 0; m < ui->feedTreeWidget->topLevelItemCount(); m++) {
            if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(m)->text(0)) {
                _currentLabel = m;
                _currentFeed = sum;
                break;
            }
            sum += ui->feedTreeWidget->topLevelItem(m)->childCount();
        }
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            QList<QString> labelList = feed->getLabel();
            foreach (QString label, labelList) {
                if (label == ui->feedTreeWidget->currentItem()->text(0)) {
                    foreach (Entry* entry, feed->getEntriesList()) {
                        QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                        newEntry->setText(0, entry->getTitle());
                        newEntry->setText(1, feed->getTitle());
                        newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                        newEntry->setText(3, entry->getAuthor());

                        if (!entry->isRead()) {
                            newEntry->setFont(0, _fontBold);
                            newEntry->setFont(1, _fontBold);
                            newEntry->setFont(2, _fontBold);
                            newEntry->setFont(3, _fontBold);
                        }

                        ui->entriesTreeWidget->addTopLevelItem(newEntry);
                    }
                }
            }
        }
    }
    //po kliknuti na zdroj
    else {
        for (int m = 0; m < ui->feedTreeWidget->topLevelItemCount(); m++) {
            if (ui->feedTreeWidget->currentItem()->parent()->text(0) == ui->feedTreeWidget->topLevelItem(m)->text(0)) {
                for (int n = 0; n < ui->feedTreeWidget->currentItem()->parent()->childCount(); n++) {
                    if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(m)->child(n)->text(0)) {
                        _currentLabel = m;
                        _currentFeed = n + sum;
                        break;
                    }
                }
            }
            sum += ui->feedTreeWidget->topLevelItem(m)->childCount();
        }
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            if (feed->getTitle() == ui->feedTreeWidget->currentItem()->text(0)) {
                if (feed->getEntriesList().count() == 0) {
                    _lastEntriesCount = ui->entriesTreeWidget->topLevelItemCount();
                    _googleReaderController->getSpecifiedNumberOfEntriesFromFeed(feed->getId(), feed->getEntriesList().count()+5);
                }
                if (feed->getUnreadCount() > feed->getEntriesList().count()) {
                    _lastEntriesCount = ui->entriesTreeWidget->topLevelItemCount();
                    _googleReaderController->getSpecifiedNumberOfEntriesFromFeed(feed->getId(), feed->getUnreadCount());
                }
                foreach (Entry* entry, feed->getEntriesList()) {
                    QTreeWidgetItem *newEntry = new QTreeWidgetItem;
                    //nastavim informace, o polozce
                    newEntry->setText(0, entry->getTitle());
                    newEntry->setText(1, feed->getTitle());
                    newEntry->setData(2, Qt::DisplayRole, entry->getPublishedDate());
                    newEntry->setText(3, entry->getAuthor());
                    newEntry->setText(4, entry->getId());
                    if (!entry->isRead()) {
                        newEntry->setFont(0, _fontBold);
                        newEntry->setFont(1, _fontBold);
                        newEntry->setFont(2, _fontBold);
                        newEntry->setFont(3, _fontBold);
                    }
                    ui->entriesTreeWidget->addTopLevelItem(newEntry);
                }
                break;
            }
        }
    }

    ui->entriesTreeWidget->setCurrentItem(ui->entriesTreeWidget->topLevelItem(-1));

    differentiateEntriesLines();
}

/*!
\brief Otevre polozku v externim prohlizeci
*/
void GooDer::openEntryInExternalBrowser() {

    QString entryID = ui->entriesTreeWidget->currentItem()->text(4);

    QString entryLink = _googleReaderController->getEntryLink(entryID);

    if (QDesktopServices::openUrl(entryLink)) {
        _googleReaderController->markEntryAsRead(entryID);
        //nastavim barvu pozadi na puvodni aby bylo patrne, ze je polozka prectena
        markUIEntryAsRead();
        checkEntriesFeedsNumbers();
    }
    else {
        showStatusBarMessage("Can't open entry in external browser!");
    }
}

/*!
\brief Zobrazi zvolenou polozku v prohlizeci a oznaci ji jako prectenou
*/
void GooDer::showSelectedEntry(QModelIndex inputIndex) {

    QString entryId = ui->entriesTreeWidget->currentItem()->text(4);

    Entry* entry = _googleReaderController->getEntry(entryId);

    //v prohlizeci otevru danou adresu
    (_showSummary) ? ui->browser->setHtml(entry->getSummary()) : ui->browser->load(QUrl(entry->getLink()));

    if (_autoHideFeedPanel)
        ui->feedTreeWidget->hide();

    ui->browser->setFocus();

    if (!entry->isRead()) {
        //zavolam metodu pro odeslani pozadavku na oznaceni
        _googleReaderController->markEntryAsRead(entry->getId());
        //nastavim barvu pozadi na puvodni aby bylo patrne, ze je polozka prectena
        markUIEntryAsRead();
    }
    else {
        if (inputIndex.row() == ui->entriesTreeWidget->topLevelItemCount()-1) {
            QString feedId = _googleReaderController->getFeedIdForEntry(entryId);
            _lastEntriesCount = ui->entriesTreeWidget->topLevelItemCount();
            _fetchingEntriesFromHistory = true;
            _fetchingEntriesFromHistoryActivated = false;
            _googleReaderController->getSpecifiedNumberOfEntriesFromFeed(feedId, ui->entriesTreeWidget->topLevelItemCount()+5);
        }
    }
    //pokud jsou ve zdroji jiz vsechny zpravy precteny zmenim barvu zdroje
    checkEntriesFeedsNumbers();

}

void GooDer::showLoadProgress(int value) {
    _loadProgress->clear();
    _loadProgress->setText(QString::number(value, 10).append(" %"));
}

/*!
\brief Zobrazi zvolenou zpravu v informacnim radku
*/
void GooDer::showStatusBarMessage(QString message) {

    QString time_now = QTime::currentTime().toString("hh:mm:ss").insert(0,"[").append("]  ");
    message.insert(0, time_now);
    ui->statusbar->showMessage(trUtf8(message.toAscii()), 4000);
}
// ?????????
void GooDer::noUserInfo() {
    setShortcuts();
//    if (googleReaderController->getPassword().isEmpty()) {
        showStatusBarMessage(trUtf8("Please enter login credentials!"));
        _settingsDialogInstance->show();
//    }
}

/*!
\brief Prida novy zdroj
*/
void GooDer::addNewFeed(QString link, QString name, QString label) {
    _googleReaderController->addFeed(link, name, label);
    emit checkFeeds();
}

/*!
\brief Zobrazi dialog pro pridani noveho zdroje
*/
void GooDer::showAddFeedDialog() {
    QList<QString> labelList;
    labelList.append(trUtf8("No label"));
    labelList.append(trUtf8("Add new label"));

    //ziskam seznam aktualne pouzivanych stitku
    foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
        QList<QString> tmpLabelList = feed->getLabel();
        foreach (QString label, tmpLabelList) {
            if (label != "") {
                if (!labelList.contains(label))
                    labelList.append(label);
            }
        }
    }
    _addFeedDialogInstance = new AddFeedDialog(labelList, this);
    connect (_addFeedDialogInstance,SIGNAL(dataReady(QString, QString, QString)),
            this, SLOT(addNewFeed(QString, QString, QString)));
    _addFeedDialogInstance->exec();
}

void GooDer::removeFeed() {
    if (!ui->feedTreeWidget->currentItem()->text(2).isEmpty())
        _googleReaderController->removeFeed(ui->feedTreeWidget->currentItem()->text(2));
}

/*!
\brief Vola funkci pro odstraneni zdroje
*/
void GooDer::removeFeed(int index) {
    if (ui->feedTreeWidget->topLevelItemCount() > 0)
        _googleReaderController->removeFeed(_googleReaderController->getFeedsFromDatabase().at(index)->getId());
}

/*!
\brief Zobrazi dialog pro odebrani zdroje
*/
void GooDer::showRemoveFeedDialog() {
    _removeFeedDialogInstance = new RemoveFeedDialog(_googleReaderController->getFeedsFromDatabase(), this);
    connect (_removeFeedDialogInstance,SIGNAL(removeFeedData(int)),
           this, SLOT(removeFeed(int)));
    _removeFeedDialogInstance->exec();
}

/*!
\brief Zobrazi seznam zdroju
*/
void GooDer::showFeedTreeWidget() {
    if (_autoHideFeedPanel) {
        ui->frameShowFeedListButton->hide();
        ui->feedTreeWidget->show();
    }
}

/*!
\brief Minimalizuje/maximalizuje okno pri poklikani na ikonu v systemove liste
*/
void GooDer::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isMinimized()) this->showMaximized();
        else this->showMinimized();
    }
}

/*!
\brief Zobrazi oznameni o poctu novych polozek
*/
void GooDer::showNotification() {

    //vytvori ikonu
    QIcon iconNewEntry = QIcon(":/icons/icons/GooDerNew.svgz");
    QString title = "";
    QString message = "";
    int feedUnreadCount = 0;
    int totalUnreadCount = _googleReaderController->getTotalUnreadCount();

    if (totalUnreadCount <= 0)
        return;

    //tvar zpravy podle poctu polozek
    if (totalUnreadCount == 1) title = QString("%1 new item").arg(totalUnreadCount);
    else if (totalUnreadCount >= 2 && totalUnreadCount <= 4) title= QString("%1 new items").arg(totalUnreadCount);
    else if (totalUnreadCount > 4) title = QString("%1 new items").arg(totalUnreadCount);
    else title = "No new entries";

    if (totalUnreadCount > 0 && totalUnreadCount < 20) {
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            feedUnreadCount = feed->getUnreadCount();
            if (feedUnreadCount > 0)  {
                message.append("\n(").append(QString::number(feedUnreadCount)).append(")    ").append(feed->getTitle().toUtf8()).append("\n\n");
                foreach (Entry* entry, feed->getEntriesList()) {
                    if (!entry->isRead()) {
                        message.append(entry->getTitle().toUtf8()).append("\n");
                    }
                }
            }
        }
    }
    else {
        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            feedUnreadCount = feed->getUnreadCount();
            if (feedUnreadCount > 0) {
                if (feedUnreadCount < 10) message.append("(").append((QString::number(feedUnreadCount)).append(")       ").append(feed->getTitle().toUtf8().append("\n")));
                else if (feedUnreadCount < 100) message.append("(").append((QString::number(feedUnreadCount)).append(")     ").append(feed->getTitle().toUtf8().append("\n")));
                else if (feedUnreadCount < 1000) message.append("(").append((QString::number(feedUnreadCount)).append(")   ").append(feed->getTitle().toUtf8().append("\n")));
                else message.append("(").append((QString::number(feedUnreadCount)).append(")  ").append(feed->getTitle().toUtf8().append("\n")));

            }
        }
    }

    _trayIcon->setIcon(iconNewEntry);
    GooDer::setWindowIcon(iconNewEntry);

    //zobrazi "bublinovou" zpravu s obsahem
    _trayIcon->showMessage(trUtf8(title.toAscii()), trUtf8(message.toAscii()), QSystemTrayIcon::Information, 7000);
}

/*!
\brief Prejde na dalsi polozku v seznamu polozek
*/
void GooDer::readNextEntry() {
    //pokud se mam kam pohybovat
    if (ui->entriesTreeWidget->topLevelItemCount() > 0) {
        //pokud je oznacena polozka
        if (ui->entriesTreeWidget->currentIndex().row() != -1) {
            //pokud nejsem na konci seznamu
            if (ui->entriesTreeWidget->currentIndex().row() != ui->entriesTreeWidget->topLevelItemCount()-1) {
                //prejdu na dalsi polozku
                ui->entriesTreeWidget->setCurrentIndex(ui->entriesTreeWidget->indexBelow(ui->entriesTreeWidget->currentIndex()));
                emit signalReadNextEntry(ui->entriesTreeWidget->currentIndex());
            }
            //pokud jsem na konci seznamu
            else {
                //stahnu dalsich 5 plozek z databaze
                for (int x = 0; x < _googleReaderController->getFeedsFromDatabase().count(); x++) {
                    if (ui->feedTreeWidget->currentItem()->text(0) == _googleReaderController->getFeedsFromDatabase().at(x)->getTitle()) {
                        _lastEntriesCount = ui->entriesTreeWidget->topLevelItemCount();
                        _fetchingEntriesFromHistory = true;
                        _fetchingEntriesFromHistoryActivated = true;
                        _googleReaderController->getSpecifiedNumberOfEntriesFromFeed(_googleReaderController->getFeedsFromDatabase().at(x)->getId(), _googleReaderController->getFeedsFromDatabase().at(x)->entries.count()+5);
//                        googleReaderController->getNumberOfEntriesFromFeed(googleReaderController->getFeedsFromDatabase().at(x)->getId(), googleReaderController->getFeedsFromDatabase().at(x)->entries.count()+5, true);
                    }
                }
            }
        }
        //pokud neni nastavim jako aktualni prvni polozku
        else {
            QPoint point(0,0);
            QModelIndex index(ui->entriesTreeWidget->indexAt(point));
            ui->entriesTreeWidget->setCurrentIndex(index);
            emit signalReadNextEntry(index);
        }
    }
}

/*!
\brief Prejde na predchozi polozku v seznamu polozek
*/
void GooDer::readPreviousEntry() {
    //pokud se mam kam pohybovat
    if (ui->entriesTreeWidget->topLevelItemCount() > 0) {
        //a pokud je zvolena polozka
        if (ui->entriesTreeWidget->currentIndex().row() != -1) {
            //a pokud nejsem na zacatku
            if (ui->entriesTreeWidget->currentIndex().row() > 0) {
                //presunu se na predchozi polozku
                ui->entriesTreeWidget->setCurrentIndex(ui->entriesTreeWidget->indexAbove(ui->entriesTreeWidget->currentIndex()));
                emit signalReadPreviousEntry(ui->entriesTreeWidget->currentIndex());
            }
        }
    }
}

/*!
\brief Prejde na dalsi zdroj
*/
void GooDer::readNextFeed() {

    int sumOfFeeds = 0;

    //pokud se mam kam pohybovat
    if (ui->feedTreeWidget->topLevelItemCount() > 0) {
        //spocitam si celkovy pocet zdroju v panelu
        for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
            sumOfFeeds += ui->feedTreeWidget->topLevelItem(i)->childCount();
        }
        //pokud je aktualne oznacena polozka
        if (ui->feedTreeWidget->currentIndex().row() != -1) {
            //pokud nejsem na konci seznamu
            if (_currentFeed < sumOfFeeds-1) {
                //posunu se
                _currentFeed++;
                for (int j = 0; j < ui->feedTreeWidget->topLevelItemCount(); j++) {
                    //pokud je dalsi polozkou stitek preskocim jej
                    if (ui->feedTreeWidget->itemBelow(ui->feedTreeWidget->currentItem())->text(0) == ui->feedTreeWidget->topLevelItem(j)->text(0)) {
                       ui->feedTreeWidget->setCurrentIndex(ui->feedTreeWidget->indexBelow(ui->feedTreeWidget->currentIndex()));
                       _currentLabel++;
                    }
                }
                ui->feedTreeWidget->setCurrentIndex(ui->feedTreeWidget->indexBelow(ui->feedTreeWidget->currentIndex()));
                emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
            }
        }
        //pokud neni oznacena polozka presunu se na prvni
        else {
            _currentFeed++;
            ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->itemAt(0, 20));
            emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
        }
    }
}

/*!
\brief Prejde na predchozi zdroj
*/
void GooDer::readPreviousFeed() {

    bool goOverLabelBack = false;

    //pokud se mam kde pohybovat
    if (ui->feedTreeWidget->topLevelItemCount() > 0 && ui->feedTreeWidget->currentIndex().row() != -1) {
        //pokud nejsem na zacatku
        if (ui->feedTreeWidget->currentItem()->text(0) != ui->feedTreeWidget->topLevelItem(0)->text(0)) {
            //pokud se mam kam pohybovat
            if (ui->feedTreeWidget->currentIndex().row() != -1 && _currentFeed > 0) {
                    //zjistim jestli jsem na stitku
                    for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
                        //pokud ano
                        if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(i)->text(0)) {
                            //pokud je predchozi polozka stitek preskocim jej
                            goOverLabelBack = true;
                            _currentLabel--;
                        }
                    }
                    //posunu se na predchozi zdroj
                    if (!goOverLabelBack) _currentFeed--;
                    for (int j = 0; j < ui->feedTreeWidget->topLevelItemCount(); j++) {
                        if (ui->feedTreeWidget->itemAbove(ui->feedTreeWidget->currentItem())->text(0) == ui->feedTreeWidget->topLevelItem(j)->text(0)) {
                            //nastavim novou aktualni polozku
                            ui->feedTreeWidget->setCurrentIndex(ui->feedTreeWidget->indexAbove(ui->feedTreeWidget->currentIndex()));
                            _currentLabel--;
                        }
                    }
                    ui->feedTreeWidget->setCurrentIndex(ui->feedTreeWidget->indexAbove(ui->feedTreeWidget->currentIndex()));
                    emit signalReadPreviousFeed(ui->feedTreeWidget->currentIndex());
            }
        }
    }
}


/*!
\brief Prejde na nasledujici stitek
*/
void GooDer::readNextLabel() {
    //pokud se mam kam pohybovat
    if (ui->feedTreeWidget->currentIndex().row() != -1 && ui->feedTreeWidget->topLevelItemCount() > 0) {
        bool breaking = false;
        bool onFeed = false;
        //zjistim jestli se nachazim na stitku nebo na zdroji
        for (int x = _currentLabel; x < ui->feedTreeWidget->topLevelItemCount(); x++) {
            if (ui->feedTreeWidget->currentItem()->text(0) == ui->feedTreeWidget->topLevelItem(x)->text(0)) {
                onFeed = true;
            }
        }
        //pokud nejsem na poslednim stitku
        if (ui->feedTreeWidget->topLevelItemCount()-1 != _currentLabel) {
            if (ui->feedTreeWidget->currentItem()->text(0) != ui->feedTreeWidget->topLevelItem(ui->feedTreeWidget->topLevelItemCount()-1)->text(0)) {
                //pokud jsem na zdroji
                if (onFeed) {
                    for (int i = _currentLabel; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
                        for (int j = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
                            //prejdu na nasledujici stitek
                            if (ui->feedTreeWidget->itemBelow(ui->feedTreeWidget->currentItem())->text(0) == ui->feedTreeWidget->topLevelItem(i)->text(0)) {
                                _currentLabel++;
                                breaking = true;
                                break;
                            }
                            else if (!breaking) {
                                ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->itemBelow(ui->feedTreeWidget->currentItem()));
                                _currentFeed++;
                            }
                        }
                    }
                }
                //pokud jsem na stitku
                else {
                    for (int i = _currentLabel; i < ui->feedTreeWidget->topLevelItemCount()-1; i++) {
                        for (int j = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
                            //prejdu na dalsi stitek
                            if (ui->feedTreeWidget->itemBelow(ui->feedTreeWidget->currentItem())->text(0) == ui->feedTreeWidget->topLevelItem(i+1)->text(0)) {
                                _currentLabel++;
                                breaking = true;
                                break;
                            }
                            else if (!breaking) {
                                ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->itemBelow(ui->feedTreeWidget->currentItem()));
                                _currentFeed++;
                            }
                        }
                    }
                }
                ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->topLevelItem(_currentLabel));
                emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
            }
        }
    }
    //prejdu na prvni stitek Vsechny polozky
    else {
        ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->itemAt(0, 0));
        emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
    }
}

/*!
\brief Prejde na predchozi stitek
*/
void GooDer::readPreviousLabel() {
    //pokud mam kam jit
    if (ui->feedTreeWidget->topLevelItemCount() > 0 && ui->feedTreeWidget->currentIndex().row() != -1) {
        //a pokud nejsem na zacatku seznamu zdroju
        if (ui->feedTreeWidget->currentItem()->text(0) != ui->feedTreeWidget->topLevelItem(0)->text(0)) {
            //pokud jsem na jinem stitku
            if (_currentLabel > 0) {
                //presunu se
                _currentLabel--;
                ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->topLevelItem(_currentLabel));
                _currentFeed -= ui->feedTreeWidget->currentItem()->childCount();
                //zobrazim polozky
                emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
            }
            else {
                //presunu se na stitek vsechny polozky
                ui->feedTreeWidget->setCurrentItem(ui->feedTreeWidget->topLevelItem(0));
                emit signalReadNextFeed(ui->feedTreeWidget->currentIndex());
            }
        }
    }
}

/*!
\brief Zobrazi/skryje panel nastroju
*/
void GooDer::showHideToolbar() {
    (ui->toolBar->isHidden()) ? ui->toolBar->show() : ui->toolBar->hide();
}

/*!
\brief Zobrazi/skryje menu
*/
void GooDer::showHideMenu() {
    (ui->menubar->isHidden()) ? ui->menubar->show() : ui->menubar->hide();
}

/*!
\brief Skryje panel pro hledani a zadavani prikazu
*/
void GooDer::hideSearchCommandBar() {
    if (!ui->frameSearch->isHidden()) ui->frameSearch->hide();
    ui->entriesTreeWidget->setFocus();
}

/*!
\brief Zobrazi/skryje seznam zdroju
*/
void GooDer::showHideFeedList() {
    if (ui->feedTreeWidget->isHidden()) {
      ui->feedTreeWidget->show();
      ui->frameShowFeedListButton->hide();
    }
    else {
      ui->feedTreeWidget->hide();
//      ui->frameShowFeedListButton->show();
    }
}

/*!
\brief Hleda polozky podle zadaneho slova
*/
void GooDer::searchEntry() {

    QList<Entry*> searchResult;
    //ulozime si vyhledavane slovo
    QString keyword = ui->searchKeyWord->text();

    ui->frameSearch->hide();

    //a projedeme seznam polozek a hledame vsechny polozky, ktere obsahuji hledanou frazi
    foreach (Feed * feed, _googleReaderController->getFeedsFromDatabase()) {
        foreach (Entry* entry, feed->getEntriesList()) {
            if (entry->getTitle().contains(keyword, Qt::CaseInsensitive))
                searchResult.append(entry);
        }
    }

    ui->searchKeyWord->clear();
    ui->entriesTreeWidget->clear();

    //ve statusbaru informujeme o poctu nalezenych polozek
    if (searchResult.isEmpty()) showStatusBarMessage(QString("Nothing found for: ").append(keyword));
    else showStatusBarMessage(QString("Entries found: ").append(QString::number(searchResult.count())));

    //nalezene polozky vypiseme
    foreach (Entry* entrySearched, searchResult) {
        QTreeWidgetItem *newEntry = new QTreeWidgetItem;
        QString feedName = "";

        foreach (Feed* feed, _googleReaderController->getFeedsFromDatabase()) {
            foreach (Entry* entry, feed->getEntriesList()) {
                if (entrySearched->getId() == entry->getId()) {
                    feedName = feed->getTitle();
                    break;
                }
            }
        }
        newEntry->setText(0, entrySearched->getTitle());
        newEntry->setText(1, feedName);
        newEntry->setData(2, Qt::DisplayRole, entrySearched->getPublishedDate());
        newEntry->setText(3, entrySearched->getAuthor());
        newEntry->setText(4, entrySearched->getId());
        if (!entrySearched->isRead()) {
            newEntry->setFont(0, _fontBold);
            newEntry->setFont(1, _fontBold);
            newEntry->setFont(2, _fontBold);
            newEntry->setFont(3, _fontBold);
        }

        ui->entriesTreeWidget->addTopLevelItem(newEntry);
    }

    differentiateEntriesLines();
}

/*!
  \brief Mark entry as read in GUI
 */
void GooDer::markUIEntryAsRead() {
    ui->entriesTreeWidget->currentItem()->setFont(0, _fontNormal);
    ui->entriesTreeWidget->currentItem()->setFont(1, _fontNormal);
    ui->entriesTreeWidget->currentItem()->setFont(2, _fontNormal);
    ui->entriesTreeWidget->currentItem()->setFont(3, _fontNormal);
}

void GooDer::markUIFeedAsRead(int positionLabel, int positionFeed) {
    //zdroj zvyraznim
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setFont(0, _fontNormal);
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setFont(1, _fontNormal);
    //nastavim pocet neprectenych polozek
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setText(1, QString::number(0));
}

void GooDer::markUIFeedAsUnread(int positionLabel, int positionFeed, int numberOfNewEntries) {
    //zdroj zvyraznim
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setFont(0, _fontBold);
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setFont(1, _fontBold);
    //nastavim pocet neprectenych polozek
    ui->feedTreeWidget->topLevelItem(positionLabel)->child(positionFeed)->setText(1, QString::number(numberOfNewEntries));
}

void GooDer::markAllUIEntriesListAsRead() {
    //v seznamu vsech polozek oznacim polozky jako prectene
    for (int i = 0; i < ui->entriesTreeWidget->topLevelItemCount(); i++) {
        ui->entriesTreeWidget->topLevelItem(i)->setFont(0, _fontNormal);
        ui->entriesTreeWidget->topLevelItem(i)->setFont(1, _fontNormal);
        ui->entriesTreeWidget->topLevelItem(i)->setFont(2, _fontNormal);
        ui->entriesTreeWidget->topLevelItem(i)->setFont(3, _fontNormal);
    }
}

void GooDer::markAllUIFeedsListAsRead() {
    //nastavim vzhled polozek ve sloupci se zdroji jako prectene polozky
    for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
        if (ui->feedTreeWidget->topLevelItem(i)->font(0) == _fontBold) {
            ui->feedTreeWidget->topLevelItem(i)->setText(1, "0");
            ui->feedTreeWidget->topLevelItem(i)->setFont(0, _fontNormal);
            ui->feedTreeWidget->topLevelItem(i)->setFont(1, _fontNormal);
        }
        for (int j = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
            if (ui->feedTreeWidget->topLevelItem(i)->child(j)->font(0) == _fontBold) {
                ui->feedTreeWidget->topLevelItem(i)->child(j)->setText(1, "0");
                ui->feedTreeWidget->topLevelItem(i)->child(j)->setFont(0, _fontNormal);
                ui->feedTreeWidget->topLevelItem(i)->child(j)->setFont(1, _fontNormal);
            }
        }
    }
}

void GooDer::differentiateFeedsLines() {

    for (int i = 0; i < ui->feedTreeWidget->topLevelItemCount(); i++) {
        ui->feedTreeWidget->topLevelItem(i)->setBackgroundColor(0, _labelColor);
        ui->feedTreeWidget->topLevelItem(i)->setBackgroundColor(1, _labelColor);
        ui->feedTreeWidget->topLevelItem(i)->setTextAlignment(1, Qt::AlignRight);
        for (int j = 0; j < ui->feedTreeWidget->topLevelItem(i)->childCount(); j++) {
            ui->feedTreeWidget->topLevelItem(i)->child(j)->setTextAlignment(1, Qt::AlignRight);
            if (j % 2 == 0) {
                ui->feedTreeWidget->topLevelItem(i)->child(j)->setBackground(0,_lineColor);
                ui->feedTreeWidget->topLevelItem(i)->child(j)->setBackground(1,_lineColor);
            }
        }
    }
}

void GooDer::differentiateEntriesLines() {

    for (int i = 0; i < ui->entriesTreeWidget->topLevelItemCount(); i++) {
        if (i % 2 == 0) {
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(0, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(1, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(2, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(3, _lineColor);
        }
    }
}

void GooDer::changeSortingUIChanges(int index) {

    Q_UNUSED(index);

    for (int i = 0; i < ui->entriesTreeWidget->topLevelItemCount(); i++) {

        ui->entriesTreeWidget->topLevelItem(i)->setBackground(0, Qt::white);
        ui->entriesTreeWidget->topLevelItem(i)->setBackground(1, Qt::white);
        ui->entriesTreeWidget->topLevelItem(i)->setBackground(2, Qt::white);
        ui->entriesTreeWidget->topLevelItem(i)->setBackground(3, Qt::white);

        if (i % 2 == 0) {
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(0, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(1, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(2, _lineColor);
            ui->entriesTreeWidget->topLevelItem(i)->setBackground(3, _lineColor);
        }
    }
}

void GooDer::statusLogin(bool status) {
    if (!status)
        this->setStatusBarMessage(trUtf8("Can't login to server. Please check internet connection."));
    else
        _online = true;
}

void GooDer::statusAddFeed(bool status) {
    if (status) {
        this->setStatusBarMessage(trUtf8("New feed was added."));
        this->newFeedAdded();
    }
    else
        this->setStatusBarMessage(trUtf8("New feed was not added!"));
}

void GooDer::statusRemoveFeed(bool status) {
    if (status) {
        this->setStatusBarMessage(trUtf8("Feed was removed."));
        this->s_refreshAfterFeedDeleted();
    }
    else
        this->setStatusBarMessage(trUtf8("Feed was not removed!"));
}

void GooDer::statusMarkFeedAsRead(bool status) {
    if (status) {
        this->setStatusBarMessage(trUtf8("Feed was marked as read."));
        this->checkUInumbers();
    }
    else
        this->setStatusBarMessage(trUtf8("Feed was not marked as read!"));
}

void GooDer::statusMarkEntryAsRead(bool status) {
    if (status)
        this->setStatusBarMessage(trUtf8("Entry was marked as read."));
    else
        this->setStatusBarMessage(trUtf8("Entry was not marked as read!"));
}

void GooDer::statusAddFeedLabel(bool status) {
    if (status) {
        this->setStatusBarMessage(trUtf8("Label was added."));
        this->s_refreshFeedWidget();
    }
    else
        this->setStatusBarMessage(trUtf8("Label was not added!"));
}

void GooDer::statusRemoveFeedLabel(bool status) {
    if (status) {
        this->setStatusBarMessage(trUtf8("Label was removed."));
        this->s_refreshFeedWidget();
    }
    else
        this->setStatusBarMessage(trUtf8("Label was not removed!"));
}

void GooDer::statusGetEntries(bool status) {
    if (status)
        this->getEntriesReady();
}
