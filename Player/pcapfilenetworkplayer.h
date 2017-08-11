#ifndef PCAPFILENETWORKPLAYER_H
#define PCAPFILENETWORKPLAYER_H

#include "player.h"
#include "../Configuration/workerconfiguration.h"
#include "../PacketProducer/pcapbufferedproducer.h"
#include "../PacketConsumer/pcapnetworkconsumer.h"
#include "../Middleware/analyzerpcapmiddleware.h"
#include "../Middleware/looppcapmiddleware.h"
#include "Status/analyzerstatus.h"
#include "Status/finalstatus.h"

class PcapFileNetworkPlayer : public Player
{
    Q_OBJECT
private:
    PcapBufferedProducer producer;
    PcapNetworkConsumer consumer;
    AnalyzerPcapMiddleware analyzerMiddleware;
    LoopPcapMiddleware loopMiddleware;

public:
    explicit PcapFileNetworkPlayer(WorkerConfiguration config, QObject *parent = 0) :
        Player(config, parent),
        producer(config),
        consumer(config),
        analyzerMiddleware(config),
        loopMiddleware(config) {}
    void start();
    void stop();
    void stopAndWait();

private slots:
    void gotProducerStatus(Status pStatus);
    void gotAnalyzerStatus(AnalyzerStatus aStatus);
    void gotConsumerStatus(Status cStatus);
    void gotProducerWorkerStatus(WorkerStatus worker);
    void moduleFinished();
};

#endif // PCAPFILENETWORKPLAYER_H
