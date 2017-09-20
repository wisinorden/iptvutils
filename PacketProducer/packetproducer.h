#ifndef PACKETPRODUCER_H
#define PACKETPRODUCER_H

#include <QObject>
#include "product.h"
#include "../Configuration/workerconfiguration.h"
#include "Status/status.h"
#include "Status/workerstatus.h"


class PacketProducer : public QObject
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    bool stopping = false;
    QHash<quint64, StreamInfo> streams;

public:
    PacketProducer(WorkerConfiguration config) : config(config) {}
    ~PacketProducer() {}
    virtual void init(QThread *thread) = 0;

signals:
    void started();
    void finished();
    void status(Status status);
    void workerStatus(WorkerStatus status);

public slots:
    virtual void stop() { stopping = true; }
    //virtual Product getProduct() = 0;

};

Q_DECLARE_INTERFACE(PacketProducer, "PacketProducer")

#endif // PACKETPRODUCER_H
