#ifndef NETWORKJITTER_H
#define NETWORKJITTER_H


#include "pcapmiddleware.h"
#include "concurrentqueue.h"
#include "Status/analyzerstatus.h"
#include "pidinfo.h"
#include "Analyzer/tsanalyzer.h"
#include "Analyzer/tserrors.h"
#include "Status/streaminfo.h"

#include <QMap>
#include <QHash>

class NetworkJitter : public PcapMiddleware
{
    Q_OBJECT
private:
    ConcurrentQueue<PcapProduct> buffer;
    qint64 duration;
    qint64 packetNumber;
    QHash<quint64, StreamInfo> streams;
    bool signalType;

public:
    NetworkJitter(WorkerConfiguration config) :
        PcapMiddleware(config),
        buffer(MIDDLEWARE_BUFFER_SIZE)
    {
        init();
    }
    void init();
    PcapProduct getProduct();

signals:
    void status(AnalyzerStatus status);
    void iatStatus(double iatDev, qint64 duration);


protected slots:
    void run();

public slots:
    void start();
    void stop();
};

#endif // NETWORKJITTER_H
