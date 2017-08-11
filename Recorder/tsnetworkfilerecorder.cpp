#include "tsnetworkfilerecorder.h"

void TsNetworkFileRecorder::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addToEnd(&analyzerMiddleware)->addToEnd(&consumer);
    producer.init(&producerThread);

    connect(&consumer, &TsFileConsumer::finished, &producer, &PcapBufferedProducer::stop);

    // Performs waiting for all modules to finish before emitting finished
    connect(&producerThread, &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);
    connect(&consumerThread, &QThread::finished, this, &TsNetworkFileRecorder::moduleFinished);

    connect(&producer, &PcapBufferedProducer::status, this, &TsNetworkFileRecorder::gotProducerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::status, this, &TsNetworkFileRecorder::gotAnalyzerStatus);
    connect(&consumer, &TsFileConsumer::status, this, &TsNetworkFileRecorder::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &TsNetworkFileRecorder::workerStatus);

    producerThread.start();
    analyzerMiddleware.start();
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

void TsNetworkFileRecorder::gotConsumerStatus(Status cStatus) {
    if (cStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(cStatus.getError());
        stop();
        emit status(finalStatus);
        return;
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
