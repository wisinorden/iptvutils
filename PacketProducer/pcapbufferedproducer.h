#ifndef PCAPFILEBUFFEREDPRODUCER_H
#define PCAPFILEBUFFEREDPRODUCER_H

#include "packetproducer.h"
#include "productprovider.h"
#include "pcapproduct.h"
#include "concurrentqueue.h"
#include "Configuration/workerconfiguration.h"
#include "Middleware/tsparser.h"
#include <QElapsedTimer>
#include <QFile>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>

extern "C" {
    #include <pcap.h>
}

class PcapBufferedProducer : public PacketProducer, public ProductProvider
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
    Product getProduct();

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
    void networkSocketStatusUpdate();
#endif

private:
    int bufferFromFileSetup();
    void bufferFromFileRun();
    void bufferFromFileTeardown();
    int bufferFromNetworkSetup();
    void bufferFromNetworkRun();
    void bufferFromNetworkTeardown();
    void networkSocketReadout();

    ConcurrentQueue<Product> buffer;

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
