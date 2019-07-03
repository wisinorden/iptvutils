#include "tsnetworkfilerecorder.h"

void TsNetworkFileRecorder::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addNext(&networkJitter)->addNext(&analyzerMiddleware)->addNext(&consumer);

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
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::bitrateStatus, this, &TsNetworkFileRecorder::gotBitrate);
    connect(&networkJitter, &NetworkJitter::workerStatus, this, &TsNetworkFileRecorder::gotIatDev);






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

void TsNetworkFileRecorder::joinStreamInfo(WorkerStatus status, bool isDeviationSignal) {
    qint8 counter = 0;

    for(auto iter = status.getStreams().begin(); iter != status.getStreams().end(); ++iter) {
        qint64 streamID = iter.key();
        const StreamInfo &streamInfo= iter.value();

        //     if(!isDeviationSignal){
        previousAnalyzerStream.streams[streamID] = streamInfo;

        // Make IAT work for any numer of streams
        if(iatVector.size() <= counter){
            previousAnalyzerStream.streams[streamID].iatDeviation = 0;

        } else {
            previousAnalyzerStream.streams[streamID].iatDeviation = (iatVector[counter].last() );
        }
        WorkerStatus completeSignal;

        completeSignal.setStreams(previousAnalyzerStream.streams);
        emit workerStatus(completeSignal);
        counter++;
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

void TsNetworkFileRecorder::gotBitrate(double  bitrate, qint64 duration){

    emit bitrateStatus(bitrate, duration);
}



void TsNetworkFileRecorder::gotIatDev(WorkerStatus status, bool isDeviationSignal){

    this->iatVector.resize(status.streams.count());

    for(int i = 0; i < status.streams.count(); i++) {
        quint64 hashKey = status.streams.keys().at(i);
        //  qInfo() << (status.streams[hashKey].iatDeviation)<< "IATVALUE BEING APPENDED";

        iatVector[i].append(status.streams[hashKey].iatDeviation);
        //qInfo() << iatDevList.last() << "IATLIST VALUE AT INDEX 0";
    }
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
