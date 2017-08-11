#ifndef PIDINFO_H
#define PIDINFO_H

#include <QtNumeric>

class PIDInfo {
public:
    PIDInfo() {}

    quint8 cc = 0;
    quint64 ccErrors = 0;
    quint8 scramble = 0;
    quint64 lastArrived = 0;
};

#endif // PIDINFO_H
