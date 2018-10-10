#ifndef ANALYZERTSMIDDLEWARE_H
#define ANALYZERTSMIDDLEWARE_H

#include "tsmiddleware.h"
#include "concurrentqueue.h"
#include "Status/analyzerstatus.h"
#include "pidinfo.h"
#include "Analyzer/tsanalyzer.h"
#include "Analyzer/tserrors.h"
#include "Status/streaminfo.h"
#include <QMap>
#include <QHash>

class AnalyzerTsMiddleware : public TsMiddleware
{
private:
    ConcurrentQueue<Product> buffer;
    TsParser tsParser;
    TsAnalyzer tsAnalyzer;
    TsErrors tsErrors;
    QMap<int, PIDInfo> pidMap;
    qint64 packetNumber;
    qint64 duration;
    void bufferProducts();
    QHash<quint64, StreamInfo> streams;
public:
    AnalyzerTsMiddleware(WorkerConfiguration config) :
        TsMiddleware(config),
        buffer(MIDDLEWARE_BUFFER_SIZE),
        tsParser(),
        tsAnalyzer(tsParser, duration, packetNumber)
    {
        init();
    }
    void init();
    Product getProduct();

signals:
    void status(AnalyzerStatus status);

protected slots:
    void run();

public slots:
    void start();
    void stop();
};

#endif // ANALYZERTSMIDDLEWARE_H
