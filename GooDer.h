#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "googlereadercontroller.h"
#include "simplecrypt.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QtNetwork/QNetworkCookie>
#include <QModelIndex>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTreeWidget>
#include <QFile>
#include <QHeaderView>
#include <QSettings>

class AddFeedDialog;
class RemoveFeedDialog;
class SettingsDialog;
class LabelNameDialog;

namespace Ui {
    class MainWindow;
}

class GooDer : public QMainWindow {
    Q_OBJECT

    public:
        GooDer(QWidget *parent = 0);
        ~GooDer();
        void initialize();

    protected:
        void changeEvent(QEvent *e);
        bool eventFilter(QObject*, QEvent*);

    signals:
        void setStatusBarMessage(QString);
        void checkFeeds();
        void signalShowSettingsDialog();
        void signalShowAddFeedDialog();
        void signalShowRemoveFeedDialog();
        void signalAddFeed(QString, QString, QString);
        void signalReadNextEntry();
        void signalReadNextEntry(QModelIndex);
        void signalReadPreviousEntry();
        void signalReadPreviousEntry(QModelIndex);
        void signalReadNextFeed();
        void signalReadNextFeed(QModelIndex);
        void signalReadPreviousFeed();
        void signalReadPreviousFeed(QModelIndex);
        void signalReadNextLabel();
        void signalReadPreviousLabel();
        void signalLogin();
        void signalShowHideToolbar();
        void signalShowHideMenu();
        void signalRemoveLabel();
        void signalAddLabel(QString);

    private slots:
        void iconActivated(QSystemTrayIcon::ActivationReason reason);
        void showStatusBarMessage(QString);
        void showAddFeedDialog();
        void addNewFeed(QString, QString, QString);
        void showRemoveFeedDialog();
        void callCheckFeeds();
        void getEntriesReady();
        void showEntriesOnSelectedFeed(QModelIndex);
        void showSelectedEntry(QModelIndex);
        void trayActivated(QSystemTrayIcon::ActivationReason);
        void showNotification();
        void removeFeed(int);
        void removeFeed();
        void markAsRead();
        void showFeedTreeWidget();
        void showSettingsDialog();
        void showSettingsDialogSuccess();
        void login();
        void showCreateNewLabelDialog();
        void createNewLabel(QString);
        void showCommandLine();
        void parseCommands();
        void openEntryInExternalBrowser();
        void showSearchLine();
        void readNextEntry();
        void readPreviousEntry();
        void readNextFeed();
        void readPreviousFeed();
        void readNextLabel();
        void readPreviousLabel();
        void noUserInfo();
        void getEntriesFromFeed(bool moveToNextEntry);
        void showHideToolbar();
        void hideSearchCommandBar();
        void showHideFeedList();
        void searchEntry();
        void onlineState(bool);
        void showHideMenu();
        void removeLabel();
        void showAbout();
        void showHelp();
        void setFlash(bool);
        void showLoadProgress(int);
        void trayMarkAllAsRead();
        void newFeedAdded();
        void loginSuccessfull(bool);
        void checkUInumbers();
        void changeSortingUIChanges(int);
        void s_refreshFeedWidget();
        void s_refreshAfterFeedDeleted();

        void statusLogin(bool);
        void statusGetEntries(bool);
        void statusMarkFeedAsRead(bool);
        void statusMarkEntryAsRead(bool);
        void statusAddFeedLabel(bool);
        void statusRemoveFeedLabel(bool);
        void statusAddFeed(bool);
        void statusRemoveFeed(bool);

    private:
        Ui::MainWindow *ui;

        GoogleReaderController* _googleReaderController;
        AddFeedDialog* _addFeedDialogInstance;
        RemoveFeedDialog* _removeFeedDialogInstance;
        SettingsDialog* _settingsDialogInstance;
        LabelNameDialog* _labelNameDialogInstance;
        QSettings* _settings;
        SimpleCrypt* _crypt;

        QSystemTrayIcon* _trayIcon;
        QTimer* _timerRefreshFeeds;
        QMenu* _trayMenu;
        QMenu* _feedContextMenu;
        QMenu* _entryContextMenu;
        QAction* _actionCreateNewLabel;
        QAction* _actionRemoveLabel;
        QAction* _actionRemoveFeed;
        QAction* _actionOpenInExternBrowser;
        QStringList _commandHistory;
        QList<QString> _shortcuts;

        QLineEdit* _commandsLine;
        QLineEdit* _loadProgress;

        QString _shortcutNextEntry;
        QString _shortcutPrevEntry;
        QString _shortcutNextFeed;
        QString _shortcutPrevFeed;
        QString _shortcutNextLabel;
        QString _shortcutPrevLabel;
        QString _shortcutToggleFeedPanel;

        QFont _fontBold;
        QFont _fontNormal;
        QColor _labelColor;
        QColor _lineColor;

        int _commandHistoryCounter;
        int _checkFeedTime;
        int _currentLabel;
        int _lastEntriesCount;

        bool _online;
        bool _autoHideFeedPanel;
        bool _flashEnabled;
        bool _showSummary;
        bool _feedAdded;
        bool _firstRun;
        bool _showAllFeeds;
        bool _showLabels;
        bool _fetchingEntriesFromHistory;
        // true if activated from keyboard shortcut, false if with click
        bool _fetchingEntriesFromHistoryActivated;

        void connectSignals();
        void connectSignalsUI();
        void connectSignalsAccount();
        void connectSignalsNetwork();
        void connectSignalsControl();
        void setIcon();
        void createTray();
        void checkEntriesFeedsNumbers();
        void markLabelAsRead();
        void markFeedAsRead();
        void markAllUIEntriesListAsRead();
        void markAllUIFeedsListAsRead();
        void markAllAsRead();
        void saveSettings();
        void loadSettings();
        void createFeedsContextMenu();
        void createEntriesContextMenu();
        void setShortcuts();
        void setFeedListWidth();
        void setStatusbarWidgets();
        void differentiateEntriesLines();
        void differentiateFeedsLines();
        void markUIEntryAsRead();
        void markUIEntryAsUnread();
        void markUIFeedAsRead(int, int);
        void markUIFeedAsUnread(int, int, int);
        void setEntriesWidget();
        void refreshFeedWidget();
        void addFeedEntriesToEntriesList(Feed*);
        void toggleLabelsVisibility(bool);
};

#endif // MAINWINDOW_H
