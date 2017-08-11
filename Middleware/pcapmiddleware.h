#ifndef PCAPMIDDLEWARE_H
#define PCAPMIDDLEWARE_H

#include <QThread>
#include "PacketProducer/pcapproductprovider.h"
#include "Configuration/workerconfiguration.h"
#include "Status/workerstatus.h"

#define MIDDLEWARE_BUFFER_SIZE 16

class PcapMiddleware : public QObject, public PcapProductProvider
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    bool stopping = false;
    QThread runnerThread;

public:
    PcapMiddleware(WorkerConfiguration config) :
        QObject(), config(config) {}

signals:
    void started();
    void finished();
    void workerStatus(WorkerStatus status);

protected slots:
    virtual void run() = 0;

public slots:
    virtual void start() = 0;
    virtual void stop() { stopping = true; }
};

#endif // PCAPMIDDLEWARE_H
