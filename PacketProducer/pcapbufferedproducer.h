#ifndef PCAPFILEBUFFEREDPRODUCER_H
#define PCAPFILEBUFFEREDPRODUCER_H

#include "packetproducer.h"
#include "pcapproductprovider.h"
#include "pcapproduct.h"
#include "concurrentqueue.h"
#include "Configuration/workerconfiguration.h"
#include <QElapsedTimer>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>

extern "C" {
    #include <pcap.h>
}

class PcapBufferedProducer : public PacketProducer, public PcapProductProvider
{
    Q_OBJECT
    Q_INTERFACES(PacketProducer)

public:
    //PcapFileBufferedProducer(QObject *parent = 0) : PacketProducer(parent) {}
    PcapBufferedProducer(WorkerConfiguration config, int bufferSize = 1024) :
        PacketProducer(config),
        buffer(bufferSize),
        bytes(0),
        statusLastBytes(0)
    {}
    ~PcapBufferedProducer() {}
    void init(QThread *thread);
    PcapProduct getProduct();

public slots:
    void stop();

signals:
#ifndef Q_OS_WIN
    void _internalStopSignal();
#endif

private slots:
    void setup();

#ifndef Q_OS_WIN
    void _internalStop();
    void networkSocketActivated(int socket);
    void networkSocketTimeout();
#endif
    void networkSocketStatusUpdate();

private:
    int bufferFromFileSetup();
    void bufferFromFileRun();
    void bufferFromFileTeardown();
    int bufferFromNetworkSetup();
    void bufferFromNetworkRun();
    void bufferFromNetworkTeardown();
    void networkSocketReadout();

    ConcurrentQueue<PcapProduct> buffer;

    pcap_t *pcapHandle;

#ifndef Q_OS_WIN
    QSocketNotifier *networkSocketNotifier;
    QTimer *networkSocketTimer;
    QTimer *networkSocketStatusTimer;
#endif
    QElapsedTimer elapsedTimer;
    qint64 bytes;
    qint64 statusLastBytes;
    qint64 statusLastTime;

    char pcap_errbuf[PCAP_ERRBUF_SIZE];
};

#endif // PCAPFILEBUFFEREDPRODUCER_H
