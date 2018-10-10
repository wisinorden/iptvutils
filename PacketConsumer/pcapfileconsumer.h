#ifndef PCAPFILECONSUMER_H
#define PCAPFILECONSUMER_H

#include "packetconsumer.h"
#include "../PacketProducer/pcapbufferedproducer.h"
#include "PacketProducer/productprovider.h"

extern "C" {
    #include <pcap.h>
}

class PcapFileConsumer : public PacketConsumer, public ProductProvider
{
private:
    void analysisMode();
    void saveToFile();

public:
    PcapFileConsumer(WorkerConfiguration config) : PacketConsumer(config) {}
    void start(QThread *thread);
    void stop();

private slots:
    void run();
};

#endif // PCAPFILECONSUMER_H
