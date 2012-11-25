#include <QApplication>
#include "GooDer.h"
#include <QDesktopServices>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

void myMessageHandler(QtMsgType type, const char *msg)
{
    QString time_now = QDateTime::currentDateTime().toString("dd.MM hh:mm:ss").insert(0,"[").append("]  ");

    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%1 Debug: %2").arg(time_now, msg);
        break;
    case QtWarningMsg:
        txt = QString("%1 Warning: %2").arg(time_now, msg);
    break;
    case QtCriticalMsg:
        txt = QString("%1 Critical: %2").arg(time_now, msg);
    break;
    case QtFatalMsg:
        txt = QString("%1 Fatal: %2").arg(time_now, msg);
        abort();
    }
    QFile outFile("log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

/*!
\brief Vytvoreni instance hlavniho okna a jeji spusteni
*/
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

/*    qInstallMsgHandler(myMessageHandler);
    qDebug() << "";
    qDebug() << "Starting";
*/
    GooDer GooDerInstance;
    GooDerInstance.showMaximized();
    GooDerInstance.initialize();
    return app.exec();
}
