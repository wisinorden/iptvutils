#include "pcapfilenetworkplayer.h"

void PcapFileNetworkPlayer::start() {
    finalStatus.setAnalysisMode(config.getWorkerMode());
    producer.addNext(&analyzerMiddleware)->addNext(&loopMiddleware)->addNext(&consumer);
    producer.init(&producerThread);

    connect(&consumer, &PcapNetworkConsumer::finished, &producer, &PcapBufferedProducer::stop);

    // Performs waiting for all modules to finish before emitting finished
    connect(&producerThread, &QThread::finished, this, &PcapFileNetworkPlayer::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &PcapFileNetworkPlayer::moduleFinished);
    connect(loopMiddleware.thread(), &QThread::finished, this, &PcapFileNetworkPlayer::moduleFinished);
    connect(&consumerThread, &QThread::finished, this, &PcapFileNetworkPlayer::moduleFinished);


    // Cleanup
    //connect(&consumer, &PcapNetworkConsumer::finished, &producer, &PcapBufferedProducer::deleteLater);
    //connect(&producerThread, &QThread::finished, &producerThread, &QThread::deleteLater);

    connect(&producer, &PcapBufferedProducer::status, this, &PcapFileNetworkPlayer::gotProducerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::status, this, &PcapFileNetworkPlayer::gotAnalyzerStatus);
    connect(&consumer, &PcapNetworkConsumer::status, this, &PcapFileNetworkPlayer::gotConsumerStatus);
    //connect(&producer, &PcapBufferedProducer::workerStatus, this, &PcapFileNetworkPlayer::gotProducerWorkerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &PcapFileNetworkPlayer::workerStatus);

    producerThread.start();
    analyzerMiddleware.start();
    loopMiddleware.start();
    consumer.start(&consumerThread);

    emit started();
}

void PcapFileNetworkPlayer::gotProducerStatus(Status pStatus) {
    if (pStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(pStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

void PcapFileNetworkPlayer::gotAnalyzerStatus(AnalyzerStatus aStatus) {
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

void PcapFileNetworkPlayer::gotConsumerStatus(Status cStatus) {
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
    else if (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE) {
        // No information is relevant from consumer
    }
}

void PcapFileNetworkPlayer::gotProducerWorkerStatus(WorkerStatus worker) {
    emit workerStatus(worker);
}

// Only emit finished when all modules are finished
void PcapFileNetworkPlayer::moduleFinished() {
    if (producerThread.isRunning())
        return;
    if(analyzerMiddleware.thread()->isRunning())
        return;
    if(loopMiddleware.thread()->isRunning())
        return;
    if(consumerThread.isRunning())
        return;

    // newline to clean up log
    qInfo("");
    emit finished();
}

void PcapFileNetworkPlayer::stop() {
    consumer.stop();
    analyzerMiddleware.stop();
    loopMiddleware.stop();
    producer.stop();
}

void PcapFileNetworkPlayer::stopAndWait() {
    stop();

    producerThread.wait();
    analyzerMiddleware.thread()->wait();
    loopMiddleware.thread()->wait();
    consumerThread.wait();
}
