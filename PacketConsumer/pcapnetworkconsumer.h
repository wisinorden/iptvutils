#ifndef PCAPNETWORKCONSUMER_H
#define PCAPNETWORKCONSUMER_H

#include "packetconsumer.h"
#include "../PacketProducer/pcapbufferedproducer.h"
#include "../Configuration/workerconfiguration.h"
#include <QThread>

extern "C" {
    #include <pcap.h>
}

class PcapNetworkConsumer : public PacketConsumer, public PcapProductProvider
{
private:
    void analysisMode();
    void playFromPcapNetwork();

public:
    PcapNetworkConsumer(WorkerConfiguration config) : PacketConsumer(config) {}
    void start(QThread *thread);
    void stop();

private slots:
    void run();
};

#endif // PCAPNETWORKCONSUMER_H
