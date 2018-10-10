#ifndef TSBUFFEREDPRODUCER_H
#define TSBUFFEREDPRODUCER_H

#include "packetproducer.h"
#include "productprovider.h"
#include "tsproduct.h"
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

class TsBufferedProducer : public PacketProducer, public ProductProvider
{
    Q_OBJECT
    Q_INTERFACES(PacketProducer)

public:
    TsBufferedProducer(WorkerConfiguration config, int bufferSize = 1024) :
        PacketProducer(config),
        buffer(bufferSize),
        bytes(0),
        statusLastBytes(0)
    {}
    ~TsBufferedProducer() {}
    void init(QThread *thread);
    Product getProduct();

public slots:
    void stop();

private slots:
    void setup();

private:
    int bufferFromFileSetup();
    void bufferFromFileRun();
    void bufferFromFileTeardown();
    ConcurrentQueue<Product> buffer;
    QElapsedTimer elapsedTimer;
    qint64 bytes;
    qint64 statusLastBytes;
    qint64 statusLastTime;
};

#endif // TSBUFFEREDPRODUCER_H
