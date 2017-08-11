#ifndef PROGRAMINFO_H
#define PROGRAMINFO_H

#include "tsparser.h"
#include <QtNumeric>

class ProgramInfo {
public:
    ProgramInfo() {}

    quint16 programNumber;
    quint16 programPid;
    quint16 PCR_PID;

};

#endif // PROGRAMINFO_H
