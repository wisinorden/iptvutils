#include "tsbufferedproducer.h"
#include "packetparser.h"
#include "igmp.h"
#include <QElapsedTimer>
#include <QSocketNotifier>

#include <fstream>
#include <iostream>

void TsBufferedProducer::init(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, SIGNAL(started()),  this, SLOT(setup()));
    connect(this, SIGNAL(finished()), thread, SLOT(quit()));
    //connect(this, SIGNAL(_internalStopSignal()), this, SLOT(_internalStop()));
}

Product TsBufferedProducer::getProduct() {
    return buffer.pop();
}

void TsBufferedProducer::stop() {
    stopping = true;
    buffer.stop();
    qInfo("stopping tsbufferedproducer");
}

void TsBufferedProducer::setup() {
    qInfo("Running TsBufferedProducer in thread: %p", QThread::currentThread());

    if (config.getInputType() == WorkerConfiguration::FILE) {
        // The filereader is blocking in function call, by the time it returns, the thread is finished
        if (bufferFromFileSetup() == 0) {
            // Enter run loop (always blocking)
            bufferFromFileRun();
        }
        else {
            bufferFromFileTeardown();
        }
    }
    else {
        qCritical("TsBufferedProducer: Unknown input type");
    }
}

int TsBufferedProducer::bufferFromFileSetup() {
    qInfo("Buffering ts from file, buffer size: %lu", buffer.max_size());

    // Inform that we are starting
    emit started();

    // Since PCAP file needs to be reopened for every loop, there is not much to do here...

    return 0;
}

void TsBufferedProducer::bufferFromFileRun()
{
    qInfo("bufferfromfilerun ts");
    QFile file(config.getFileInput().getFilename().toLocal8Bit().constData());
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo("can't open file");
        return;
    }

    QByteArray ts_pkt = 0;
    while ((ts_pkt = file.read(188)) > 0) {
        buffer.push(TsProduct(ts_pkt));
        streams[StreamId::calcId(
                    config.getNetworkOutput().getDevice().getAddress().toString(),
                    config.getNetworkOutput().getPort())]
                .bytes += 188;
    }

    buffer.push(Product(Product::END));

    emit workerStatus(WorkerStatus(WorkerStatus::STATUS_FINISHED, streams));

    bufferFromFileTeardown();
}

void TsBufferedProducer::bufferFromFileTeardown() {
    qInfo("buffer from file teardown");
    emit finished();
    QThread::currentThread()->quit();
}
