#ifndef TSNETWORKCONSUMER_H
#define TSNETWORKCONSUMER_H

#include "packetconsumer.h"
#include "../Configuration/workerconfiguration.h"
#include "../PacketProducer/tsbufferedproducer.h"
#include <QThread>

// Needed?
extern "C" {
    #include <pcap.h>
}

class TsNetworkConsumer : public PacketConsumer, public ProductProvider
{
private:
    qint64 getPcr(QByteArray t_ts_pt);
    void analysisMode();
    void playFromQNetwork();
    void playVBRFromQNetwork();
    void playFromTsNetwork();

public:
    TsNetworkConsumer(WorkerConfiguration config) : PacketConsumer(config) {}
    void start(QThread *thread);
    void stop();

private slots:
    void run();
};

#endif // TSNETWORKCONSUMER_H
