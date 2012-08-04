#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "googleAccount.h"
//#include "googleReader.h"
#include "googlereadercontroller.h"

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
//        void relogin();
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
        void getEntriesFromFeed(bool);
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
//        void statusGetFeeds(bool);
        void statusGetEntries(bool);
//        void statusGetUnreadFeeds(bool);
        void statusMarkFeedAsRead(bool);
        void statusMarkEntryAsRead(bool);
        void statusAddFeedLabel(bool);
        void statusRemoveFeedLabel(bool);
        void statusAddFeed(bool);
        void statusRemoveFeed(bool);


    private:
        Ui::MainWindow *ui;

//        GoogleReader* googleReader;
        GoogleReaderController* googleReaderController;
        AddFeedDialog* addFeedDialogInstance;
        RemoveFeedDialog* removeFeedDialogInstance;
        SettingsDialog* settingsDialogInstance;
        LabelNameDialog* labelNameDialogInstance;
        QSettings* settings;

        QSystemTrayIcon* trayIcon;
        QTimer* timerRefreshFeeds;
        QMenu* trayMenu;
        QMenu* feedContextMenu;
        QMenu* entryContextMenu;
        QAction* actionCreateNewLabel;
        QAction* actionRemoveLabel;
        QAction* actionRemoveFeed;
        QAction* actionOpenInExternBrowser;
        QStringList commandHistory;
        QList<QString> shortcuts;

        QLineEdit* commandsLine;
        QLineEdit* loadProgress;
        QLineEdit* programStatus;

        QString shortcutNextEntry;
        QString shortcutPrevEntry;
        QString shortcutNextFeed;
        QString shortcutPrevFeed;
        QString shortcutNextLabel;
        QString shortcutPrevLabel;
        QString shortcutToggleFeedPanel;

        QFont fontBold;
        QFont fontNormal;
        QColor labelColor;
        QColor lineColor;

        int colorCounter;
        int commandHistoryCounter;
        int checkFeedTime;
        int currentFeed;
        int currentLabel;
        int lastEntriesCount;
        int lastNotificationCount;
        bool online;
        bool autoHideFeedPanel;
        bool flashEnabled;
        bool showSummary;
        bool feedAdded;
        bool firstRun;
        bool showAllFeeds;

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
        QString passwordSalter(QString);
        QString passwordDeSalter(QString);
};

#endif // MAINWINDOW_H
