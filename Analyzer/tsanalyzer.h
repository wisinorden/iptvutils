#ifndef TSANALYZER_H
#define TSANALYZER_H

#include "Status/analyzerstatus.h"
#include "Middleware/tsparser.h"
#include "tserrors.h"
#include <QtNumeric>

class TsAnalyzer
{
private:
    TsErrors *tsErrors;
    TsParser &tsParser;
    QMap<int, PIDInfo> *pidMap;
    qint64 &duration;
    qint64 &packetNumber;

public:
    TsAnalyzer(TsParser &tsParser,
               qint64 &duration,
               qint64 &packetNumber) :
        tsParser(tsParser),
        duration(duration),
        packetNumber(packetNumber) {}

    void setStream(TsErrors *tsErrors, QMap<int, PIDInfo> *pidMap) {
        this->tsErrors = tsErrors;
        this->pidMap = pidMap;
    }

    bool validSyncByte();
    bool ccErrorDetect();
};

#endif // TSANALYZER_H
