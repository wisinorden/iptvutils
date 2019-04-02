#include "pcaptsconverter.h"

void PcapTsConverter::start() {
    producer.addToEnd(&analyzerMiddleware)->addToEnd(&consumer);
    producer.init(&producerThread);

    connect(&consumer, &TsFileConsumer::finished, &producer, &PcapBufferedProducer::stop);

    // Performs waiting for all modules to finish before emitting finished
    connect(&producerThread, &QThread::finished, this, &PcapTsConverter::moduleFinished);
    connect(analyzerMiddleware.thread(), &QThread::finished, this, &PcapTsConverter::moduleFinished);
    connect(&consumerThread, &QThread::finished, this, &PcapTsConverter::moduleFinished);

    connect(&producer, &PcapBufferedProducer::status, this, &PcapTsConverter::gotProducerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::status, this, &PcapTsConverter::gotAnalyzerStatus);
    connect(&consumer, &TsFileConsumer::status, this, &PcapTsConverter::gotConsumerStatus);
    connect(&analyzerMiddleware, &AnalyzerPcapMiddleware::workerStatus, this, &PcapTsConverter::workerStatus);

    producerThread.start();
    analyzerMiddleware.start();
    consumer.start(&consumerThread);

    emit started();
}

void PcapTsConverter::gotProducerStatus(Status pStatus) {
    if (pStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(pStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

void PcapTsConverter::gotAnalyzerStatus(AnalyzerStatus aStatus) {
    finalStatus.setBase(aStatus);
    finalStatus.setAnalyzerInfo(aStatus);
    emit status(finalStatus);
}

void PcapTsConverter::gotConsumerStatus(Status cStatus) {
    if (cStatus.getType() == Status::STATUS_ERROR) {
        finalStatus.setError(cStatus.getError());
        stop();
        emit status(finalStatus);
        return;
    }
}

// Only emit finished when all modules are finished
void PcapTsConverter::moduleFinished() {
    if (!producerThread.isFinished())
        return;
    if(!analyzerMiddleware.thread()->isFinished())
        return;
    if(!consumerThread.isFinished())
        return;

    emit finished();
}

void PcapTsConverter::stop() {
    consumer.stop();
    analyzerMiddleware.stop();
    producer.stop();
}

void PcapTsConverter::stopAndWait() {
    stop();

    consumerThread.wait();
    producerThread.wait();
}
