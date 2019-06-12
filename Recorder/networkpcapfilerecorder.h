#ifndef NETWORKPCAPFILERECORDER_H
#define NETWORKPCAPFILERECORDER_H

#include "recorder.h"
#include "PacketProducer/pcapbufferedproducer.h"
#include "PacketConsumer/pcapfileconsumer.h"
#include "Middleware/analyzerpcapmiddleware.h"
#include "Middleware/networkjitter.h"

class NetworkPcapFileRecorder : public Recorder
{
    Q_OBJECT
private:
    PcapBufferedProducer producer;
    PcapFileConsumer consumer;
    NetworkJitter networkJitter;
    WorkerStatus previousAnalyzerStream;

    AnalyzerPcapMiddleware analyzerMiddleware;

public:
    NetworkPcapFileRecorder(WorkerConfiguration config, QObject *parent = 0) :
        Recorder(config, parent),
        producer(config),
        consumer(config),
        networkJitter(config),
        analyzerMiddleware(config) {

    }
    ~NetworkPcapFileRecorder() { printf("DESTRUCT\n"); }

    void start();
    void stop();
    void stopAndWait();

private slots:
    void gotProducerStatus(Status pStatus);
    void gotAnalyzerStatus(AnalyzerStatus aStatus);
    void joinStreamInfo(WorkerStatus dStatus);
    void gotConsumerStatus(Status cStatus);
    void gotBitrate(double bitrate, qint64 duration);
    void gotIatDev(double iatDev, qint64 duration);

    void moduleFinished();
};

#endif // NETWORKPCAPFILERECORDER_H
