#include "tsfilenetworkplayer.h"

void TsFileNetworkPlayer::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    qInfo("Starting TsFileNetworkPlayer in thread: %p", QThread::currentThread());
    //producer.addNext(&analyzerMiddleware)->addNext(&consumer);
    producer.addNext(&consumer);
    producer.init(&producerThread);
    connect(&consumer, &TsNetworkConsumer::finished, &producer, &TsBufferedProducer::stop);
    connect(&producerThread, &QThread::finished, this, &TsFileNetworkPlayer::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &TsFileNetworkPlayer::moduleFinished);
    //connect(loopMiddleware.thread(), &QThread::finished, this, &TsFileNetworkPlayer::moduleFinished);
    connect(&consumerThread, &QThread::finished, this, &TsFileNetworkPlayer::moduleFinished);
    connect(&producer, &TsBufferedProducer::status, this, &TsFileNetworkPlayer::gotProducerStatus);
    //connect(&analyzerMiddleware, &AnalyzerTsMiddleware::status, this, &TsFileNetworkPlayer::gotAnalyzerStatus);
    connect(&consumer, &TsNetworkConsumer::status, this, &TsFileNetworkPlayer::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerTsMiddleware::workerStatus, this, &TsFileNetworkPlayer::workerStatus);
    producerThread.start();
    //analyzerMiddleware.start();
    consumer.start(&consumerThread);
    emit started();
}

void TsFileNetworkPlayer::gotProducerStatus(Status pStatus) {
    if (pStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(pStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

void TsFileNetworkPlayer::gotAnalyzerStatus(AnalyzerStatus aStatus) {
    if (aStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(aStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
    else if (config.getWorkerMode() == WorkerConfiguration::NORMAL_MODE) {
        this->finalStatus.setAnalyzerInfo(aStatus);
        emit status(finalStatus);
    }
    else if (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE) {
        this->finalStatus.setBase(aStatus);
        this->finalStatus.setAnalyzerInfo(aStatus);
        emit status(finalStatus);
    }
}

void TsFileNetworkPlayer::gotConsumerStatus(Status cStatus) {
    if (cStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(cStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
    if (config.getWorkerMode() == WorkerConfiguration::NORMAL_MODE) {
        finalStatus.setBase(cStatus);
        emit status(finalStatus);
    }
}

void TsFileNetworkPlayer::gotProducerWorkerStatus(WorkerStatus worker) {
    emit workerStatus(worker);
}

void TsFileNetworkPlayer::moduleFinished() {
    if (producerThread.isRunning()) return;
    if (analyzerMiddleware.thread()->isRunning()) return;
    if (loopMiddleware.thread()->isRunning()) return;
    if (consumerThread.isRunning()) return;
    qInfo();
    emit finished();
    //if (analyzerMiddleware.thread()->isRunning()) return;
}

void TsFileNetworkPlayer::stop() {
    qInfo("tsfilenetworkplayer stop");
    consumer.stop();
    //analyzerMiddleware.stop();
    //loopMiddleware.stop();
    producer.stop();
}

void TsFileNetworkPlayer::stopAndWait() {
    qInfo("ts stop and wait");
    stop();
    qInfo("tsfilenetworkplayer stopandwait stop() done");
    producerThread.wait();
    qInfo("tsfilenetworkplayer producerthread.wait() done");
    //analyzerMiddleware.thread()->wait();
    //loopMiddleware.thread()->wait();
    consumerThread.wait();
}
