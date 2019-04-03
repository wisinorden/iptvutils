#ifndef TSNETWORKFILERECORDER_H
#define TSNETWORKFILERECORDER_H

#include "recorder.h"
#include "PacketProducer/pcapbufferedproducer.h"
#include "PacketConsumer/tsfileconsumer.h"
#include "Middleware/analyzerpcapmiddleware.h"
#include "Middleware/networkjitter.h"

class TsNetworkFileRecorder : public Recorder
{
private:
    PcapBufferedProducer producer;
    TsFileConsumer consumer;
    AnalyzerPcapMiddleware analyzerMiddleware;
    NetworkJitter networkJitter;
    WorkerStatus previousAnalyzerStream;


public:
    TsNetworkFileRecorder(WorkerConfiguration config, QObject *parent = 0) :
        Recorder(config, parent),
        producer(config),
        consumer(config),
        analyzerMiddleware(config),
        networkJitter(config){}
    void stopAndWait();


public slots:
    void start();
    void stop();

private slots:
    void gotProducerStatus(Status pStatus);
    void gotAnalyzerStatus(AnalyzerStatus aStatus);
    void gotConsumerStatus(Status cStatus);
    void moduleFinished();
    void joinStreamInfo(WorkerStatus dStatus);
};

#endif // TSNETWORKFILERECORDER_H
