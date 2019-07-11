#include "networkpcapfilerecorder.h"

#include <QCoreApplication>
#include <QDebug>

void NetworkPcapFileRecorder::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addNext(&networkJitter)->addNext(&analyzerMiddleware)->addNext(&consumer);
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
    connect(&networkJitter, &NetworkJitter::workerStatus, this, &NetworkPcapFileRecorder::gotIatDev);


  // connect(&networkJitter, &NetworkJitter::status, this, &NetworkPcapFileRecorder::gotNetworkStatus);


    //Worker-status is connected to the right side stream info panel
    connect(&consumer, &PcapFileConsumer::status, this, &NetworkPcapFileRecorder::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &NetworkPcapFileRecorder::joinStreamInfo);
 //   connect(&networkJitter, &NetworkJitter::workerStatus, this, &NetworkPcapFileRecorder::joinStreamInfo);


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



void NetworkPcapFileRecorder::joinStreamInfo(WorkerStatus status) {
    qint8 counter = 0;

    for(auto iter = status.getStreams().begin(); iter != status.getStreams().end(); ++iter) {
        qint64 streamID = iter.key();
        const StreamInfo &streamInfo = iter.value();

        //     if(!isDeviationSignal){
        if (previousAnalyzerStream.streams.count(streamID) > 0) {
            previousAnalyzerStream.streams[streamID] = streamInfo;
        }
        // Make IAT work for any numer of streams
        if(iatVector.size() <= counter){
            previousAnalyzerStream.streams[streamID].iatDeviation = 0;

        } else {
            previousAnalyzerStream.streams[streamID].iatDeviation = (iatVector[counter].last() );
        }

        WorkerStatus completeSignal;
        completeSignal.setStreams(previousAnalyzerStream.streams);
        emit workerStatus(completeSignal);
        qInfo() << "HUR OFTA TAR SIGNALEN EMOT I NPFR" << counter;
        counter++;
    }
}

void NetworkPcapFileRecorder::gotIatDev(WorkerStatus status){

    this->iatVector.resize(status.streams.count());

    for(int i = 0; i < status.streams.count(); i++) {
        quint64 hashKey = status.streams.keys().at(i);

        iatVector[i].append(status.streams[hashKey].iatDeviation);
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

void NetworkPcapFileRecorder::gotBitrate(double  bitrate, qint64 duration){

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
