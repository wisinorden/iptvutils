#ifndef PACKETCONSUMER_H
#define PACKETCONSUMER_H

#include <QObject>
#include "../Configuration/workerconfiguration.h"
#include "Status/status.h"

class PacketConsumer : public QObject
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    bool stopping = false;

public:
    explicit PacketConsumer(WorkerConfiguration config) : config(config) {}
    virtual void start(QThread *thread) = 0;
    virtual void stop() = 0;

signals:
    void started();
    void finished();
    void status(Status status);

protected slots:
    virtual void run() = 0;
};


Q_DECLARE_INTERFACE(PacketConsumer, "PacketConsumer")

#endif // PACKETCONSUMER_H
