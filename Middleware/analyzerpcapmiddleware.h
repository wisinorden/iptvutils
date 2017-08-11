#ifndef ANALYZERPCAPMIDDLEWARE_H
#define ANALYZERPCAPMIDDLEWARE_H

#include "pcapmiddleware.h"
#include "concurrentqueue.h"
#include "Status/analyzerstatus.h"
#include "pidinfo.h"
#include "Analyzer/tsanalyzer.h"
#include "Analyzer/tserrors.h"
#include "Status/streaminfo.h"

#include <QMap>
#include <QHash>

class AnalyzerPcapMiddleware : public PcapMiddleware
{
    Q_OBJECT
private:
    ConcurrentQueue<PcapProduct> buffer;
    TsParser tsParser;
    TsAnalyzer tsAnalyzer;
    QMap<int, PIDInfo> pidMap;
    TsErrors tsErrors;
    qint64 duration;
    qint64 packetNumber;
    void bufferProducts();
    QHash<quint64, StreamInfo> streams;

public:
    AnalyzerPcapMiddleware(WorkerConfiguration config) :
        PcapMiddleware(config),
        buffer(MIDDLEWARE_BUFFER_SIZE),
        tsParser(),
        tsAnalyzer(tsParser, duration, packetNumber)
    {
        init();
    }

    void init();
    PcapProduct getProduct();

signals:
    void status(AnalyzerStatus status);

protected slots:
    void run();

public slots:
    void start();
    void stop();
};

#endif // ANALYZERPCAPMIDDLEWARE_H
