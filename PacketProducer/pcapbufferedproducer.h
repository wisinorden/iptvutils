#ifndef PCAPFILEBUFFEREDPRODUCER_H
#define PCAPFILEBUFFEREDPRODUCER_H

#include "packetproducer.h"
#include "pcapproductprovider.h"
#include "pcapproduct.h"
#include "concurrentqueue.h"
#include "Configuration/workerconfiguration.h"
#include <QThread>

extern "C" {
    #include <pcap.h>
}

class PcapBufferedProducer : public PacketProducer, public PcapProductProvider
{
    Q_OBJECT
    Q_INTERFACES(PacketProducer)

private:
    ConcurrentQueue<PcapProduct> buffer;
    pcap_t *networkHandle;
    void bufferFromFile();
    void bufferFromNetwork();

public:
    //PcapFileBufferedProducer(QObject *parent = 0) : PacketProducer(parent) {}
    PcapBufferedProducer(WorkerConfiguration config, int bufferSize = 1024) : PacketProducer(config), buffer(bufferSize) {}
    ~PcapBufferedProducer() {}
    void init(QThread *thread);
    PcapProduct getProduct();

private slots:
    void run();

public slots:
    void stop();

};

#endif // PCAPFILEBUFFEREDPRODUCER_H
