#include "tsnetworkfilerecorder.h"

void TsNetworkFileRecorder::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addNext(&analyzerMiddleware)->addNext(&networkJitter)->addNext(&consumer);

    producer.init(&producerThread);

    connect(&consumer, &TsFileConsumer::finished, &producer, &PcapBufferedProducer::stop);

    // Performs waiting for all modules to finish before emitting finished
    connect(&producerThread, &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);
    connect(networkJitter.thread(), &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);

    connect(&consumerThread, &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);

    connect(&producer, &PcapBufferedProducer::status, this, &TsNetworkFileRecorder::gotProducerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::status, this, &TsNetworkFileRecorder::gotAnalyzerStatus);
    connect(&consumer, &TsFileConsumer::status, this, &TsNetworkFileRecorder::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &TsNetworkFileRecorder::joinStreamInfo);
    connect(&networkJitter, &NetworkJitter::workerStatus, this, &TsNetworkFileRecorder::joinStreamInfo);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::bitrateStatus, this, &TsNetworkFileRecorder::gotBitrate);





    producerThread.start();
    analyzerMiddleware.start();
    networkJitter.start();
    consumer.start(&consumerThread);

    emit started();
}

void TsNetworkFileRecorder::gotProducerStatus(Status pStatus) {
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

void TsNetworkFileRecorder::gotAnalyzerStatus(AnalyzerStatus aStatus) {
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


void TsNetworkFileRecorder::joinStreamInfo(WorkerStatus xStatus) {

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

void TsNetworkFileRecorder::gotConsumerStatus(Status cStatus) {
    if (cStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(cStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

void TsNetworkFileRecorder::gotBitrate(qint64  bitrate, qint64 duration){

    emit bitrateStatus(bitrate, duration);
}



// Only emit finished when all modules are finished
void TsNetworkFileRecorder::moduleFinished() {
    if (!producerThread.isFinished())
        return;
    if(!analyzerMiddleware.thread()->isFinished())
        return;
    if(!consumerThread.isFinished())
        return;

    emit finished();
}

void TsNetworkFileRecorder::stop() {
    producer.stop();
    analyzerMiddleware.stop();
    consumer.stop();
}

void TsNetworkFileRecorder::stopAndWait() {
    stop();
    producerThread.wait();
    consumerThread.wait();
}
