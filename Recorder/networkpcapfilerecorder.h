#ifndef NETWORKPCAPFILERECORDER_H
#define NETWORKPCAPFILERECORDER_H

#include "recorder.h"
#include "PacketProducer/pcapbufferedproducer.h"
#include "PacketConsumer/pcapfileconsumer.h"
#include "Middleware/analyzerpcapmiddleware.h"

class NetworkPcapFileRecorder : public Recorder
{
    Q_OBJECT
private:
    PcapBufferedProducer producer;
    PcapFileConsumer consumer;
    AnalyzerPcapMiddleware analyzerMiddleware;

public:
    NetworkPcapFileRecorder(WorkerConfiguration config, QObject *parent = 0) :
        Recorder(config, parent),
        producer(config),
        consumer(config),
        analyzerMiddleware(config) {}

    void start();
    void stop();
    void stopAndWait();

private slots:
    void gotProducerStatus(Status pStatus);
    void gotAnalyzerStatus(AnalyzerStatus aStatus);
    void gotConsumerStatus(Status cStatus);
    void moduleFinished();
};

#endif // NETWORKPCAPFILERECORDER_H
