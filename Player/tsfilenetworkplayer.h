#ifndef TSFILENETWORKPLAYER_H
#define TSFILENETWORKPLAYER_H

#include "player.h"
#include "../Configuration/workerconfiguration.h"
#include "../PacketProducer/tsbufferedproducer.h"
#include "../PacketConsumer/tsnetworkconsumer.h"
#include "../Middleware/analyzertsmiddleware.h"
#include "../Middleware/looppcapmiddleware.h"
#include "Status/analyzerstatus.h"
#include "Status/finalstatus.h"

class TsFileNetworkPlayer : public Player
{

private:
    TsBufferedProducer producer;
    TsNetworkConsumer consumer;
    AnalyzerTsMiddleware analyzerMiddleware;
    LoopPcapMiddleware loopMiddleware;

public:
    explicit TsFileNetworkPlayer(WorkerConfiguration config, QObject *parent = 0) :
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

#endif // TSFILENETWORKPLAYER_H
