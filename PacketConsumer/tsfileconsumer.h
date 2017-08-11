#ifndef TSFILEPACKETCONSUMER_H
#define TSFILEPACKETCONSUMER_H

#include "packetconsumer.h"
#include "PacketProducer/pcapproductprovider.h"
#include "Configuration/workerconfiguration.h"
#include "PacketProducer/pcapbufferedproducer.h"
#include <QThread>

class TsFileConsumer : public PacketConsumer, public PcapProductProvider
{
private:
    void analysisMode();
    void saveToFile();

public:
    TsFileConsumer(WorkerConfiguration config) : PacketConsumer(config) {}
    void start(QThread *thread);
    void stop();

private slots:
    void run();
};

#endif // TSFILEPACKETCONSUMER_H
