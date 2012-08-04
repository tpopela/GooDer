#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QTime>

/*!
\brief Vypocte aktualni Unixovy cas
*/
long getUnixTime() {

    QDateTime UnixTime = QDateTime(QDate(1970, 1, 1));

    long seconds = UnixTime.secsTo(UnixTime.currentDateTime());

    return seconds;
}

#endif // FUNCTIONS_H
