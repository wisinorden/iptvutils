#ifndef TSMIDDLEWARE_H
#define TSMIDDLEWARE_H

#include <QThread>
#include "PacketProducer/productprovider.h"
#include "Configuration/workerconfiguration.h"
#include "Status/workerstatus.h"

#define MIDDLEWARE_BUFFER_SIZE 16

class TsMiddleware : public QObject, public ProductProvider
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    bool stopping = false;
    QThread runnerThread;

public:
    TsMiddleware(WorkerConfiguration config) :
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

#endif // TSMIDDLEWARE_H
