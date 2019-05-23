#include "networkpcapfilerecorder.h"

void NetworkPcapFileRecorder::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addNext(&analyzerMiddleware)->addNext(&networkJitter)->addNext(&consumer);
    producer.init(&producerThread);

    connect(&consumer, &PcapFileConsumer::finished, &producer, &PcapBufferedProducer::stop);

    // Performs waiting for all modules to finish before emitting finished
    connect(&producerThread, &QThread::finished, this, &NetworkPcapFileRecorder::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &NetworkPcapFileRecorder::moduleFinished);
    connect(&consumerThread, &QThread::finished, this, &NetworkPcapFileRecorder::moduleFinished);
    connect(networkJitter.thread(), &QThread::finished, this, &NetworkPcapFileRecorder::moduleFinished);

   //These take care of the bottom info panel
    connect(&producer, &PcapBufferedProducer::status, this, &NetworkPcapFileRecorder::gotProducerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::status, this, &NetworkPcapFileRecorder::gotAnalyzerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::bitrateStatus, this, &NetworkPcapFileRecorder::gotBitrate);


  // connect(&networkJitter, &NetworkJitter::status, this, &NetworkPcapFileRecorder::gotNetworkStatus);


    //Worker-status is connected to the right side stream info panel
    connect(&consumer, &PcapFileConsumer::status, this, &NetworkPcapFileRecorder::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &NetworkPcapFileRecorder::joinStreamInfo);
    connect(&networkJitter, &NetworkJitter::workerStatus, this, &NetworkPcapFileRecorder::joinStreamInfo);


    producerThread.start();
    analyzerMiddleware.start();
    networkJitter.start();
    consumer.start(&consumerThread);

    emit started();
}

void NetworkPcapFileRecorder::gotProducerStatus(Status pStatus) {
    if (pStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(pStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
    else {
        finalStatus.setBase(pStatus);
        emit status(finalStatus);
    }
}


// This function joins the signals coming in from NetworkJitter & AnalyzerPcapMiddleware and emits complete signal to be displayed in right-hand info panel
void NetworkPcapFileRecorder::joinStreamInfo(WorkerStatus xStatus) {

    if (xStatus.getType() == WorkerStatus::STATUS_ERROR) {
        stop();
        return;
    }

    for(auto iter = xStatus.getStreams().begin(); iter != xStatus.getStreams().end(); ++iter) {
        qint64 streamID = iter.key();
        const StreamInfo &streamInfo= iter.value();

        if(streamInfo.networkJitters == 0 && streamInfo.bytes != 0){
            previousAnalyzerStream.streams[streamID] = streamInfo;


        } else if(streamInfo.networkJitters != 0){
            WorkerStatus tempStatus;
            tempStatus.setStreams(previousAnalyzerStream.streams);
            tempStatus.streams[streamID].networkJitters = streamInfo.networkJitters;
            emit workerStatus(tempStatus);
        }
    }
}



void NetworkPcapFileRecorder::gotAnalyzerStatus(AnalyzerStatus aStatus) {
    if (aStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(aStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
    else {
        finalStatus.setAnalyzerInfo(aStatus);
        emit status(finalStatus);
    }
}

void NetworkPcapFileRecorder::gotBitrate(qint64  bitrate, qint64 duration){

    emit bitrateStatus(bitrate, duration);
}



void NetworkPcapFileRecorder::gotConsumerStatus(Status cStatus) {
    if (cStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(cStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

// Only emit finished when all modules are finished
void NetworkPcapFileRecorder::moduleFinished() {
    if (!producerThread.isFinished())
        return;
    if(!analyzerMiddleware.thread()->isFinished())
        return;
    if(!consumerThread.isFinished())
        return;

    emit finished();
}

void NetworkPcapFileRecorder::stop() {
    producer.stop();
    analyzerMiddleware.stop();
    consumer.stop();
}

void NetworkPcapFileRecorder::stopAndWait() {
    stop();

    consumerThread.wait();
    analyzerMiddleware.thread()->wait();
    producerThread.wait();
}
