#ifndef PCAPTSCONVERTER_H
#define PCAPTSCONVERTER_H

#include "recorder.h"
#include "Configuration/workerconfiguration.h"
#include "PacketProducer/pcapbufferedproducer.h"
#include "PacketConsumer/tsfileconsumer.h"
#include "Status/finalstatus.h"
#include "Middleware/analyzerpcapmiddleware.h"

class PcapTsConverter : public Recorder
{
    Q_OBJECT
private:
    PcapBufferedProducer producer;
    TsFileConsumer consumer;
    AnalyzerPcapMiddleware analyzerMiddleware;

public:
    PcapTsConverter(WorkerConfiguration config, QObject *parent = 0) :
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

#endif // PCAPTSCONVERTER_H
