#include "tsfileconsumer.h"
#include "packetparser.h"
#include <QElapsedTimer>
#include <QFile>
#include <QDataStream>

extern "C" {
    #include <pcap.h>
}

void TsFileConsumer::start(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &TsFileConsumer::run);
    connect(this, &TsFileConsumer::finished, thread, &QThread::quit);
    //connect(this, &TsFileConsumer::finished, &QThread::deleteLater);
    //connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void TsFileConsumer::stop() {
    stopping = true;
}

void TsFileConsumer::run() {
    emit started();
    qInfo("starting TsFileConsumer");
    switch(config.getWorkerMode()) {
        case WorkerConfiguration::ANALYSIS_MODE_OFFLINE:
        case WorkerConfiguration::ANALYSIS_MODE_LIVE:
            analysisMode();
            break;
        case WorkerConfiguration::NORMAL_MODE:
            saveToFile();
            break;
        default:
            qFatal("TsFileConsumer: Unknown WorkerMode");
    }
    qInfo("TsFileConsumer finished");
    emit finished();
    this->thread()->quit();
}

void TsFileConsumer::analysisMode() {
    PcapProduct packet = prevProvider->getProduct();
    while (!stopping && packet.type == PcapProduct::NORMAL) {
        packet = prevProvider->getProduct();
    }
}

// Pcap-packets
void TsFileConsumer::saveToFile() {
    qInfo("TsFileConsumer: Starting dump to: %s", qPrintable(config.getFileOutput().getFilename()));
    QFile dumpfile;

    dumpfile.setFileName(config.getFileOutput().getFilename());
    if (!dumpfile.open(QIODevice::WriteOnly)) {
        qCritical("TsRecorder: Could not open file to save in: %s", qPrintable(config.getFileOutput().getFilename()));
        QString error = QString(tr("Error opening output file."));
        emit status(Status(error));
        return;
    }
    QDataStream savefileStream(&dumpfile);

    PacketParser parser;
    PcapProduct packet;
    QElapsedTimer timer;
    timer.start();
    QElapsedTimer statusTimer;
    statusTimer.start();

    packet = prevProvider->getProduct();
    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    qint64 savedBytes = 0;
    qint64 lastSavedBytes = 0;

    emit status(Status(Status::STATUS_STARTED));

    while (packet.type == PcapProduct::NORMAL) {
        pkt_data = (const u_char*)packet.data.data();
        header = (pcap_pkthdr*)packet.header.data();
        parser.parse(header, pkt_data);

        if (parser.data_len % 188 != 0) {
            qInfo("TsRecorder: invalid MPEG2 Transport Stream, total size: %i", parser.data_len);
        }
        else {
            savedBytes += savefileStream.writeRawData((const char*)parser.data, parser.data_len);
        }

        if (statusTimer.elapsed() >= 1000) {
            //qInfo("TsRecord recording: %lli ms, %lli MB, bitrate: %lli Kbps", timer.elapsed(), savedBytes/1000000, ((savedBytes-lastSavedBytes)*8/1000000)/timer.elapsed());
            emit status(Status(Status::STATUS_PERIODIC, savedBytes, timer.elapsed(), ((savedBytes-lastSavedBytes)*8/1000000)/timer.elapsed()));
            lastSavedBytes = savedBytes;
            statusTimer.restart();
        }

        packet = prevProvider->getProduct();
    }
    emit status(Status(Status::STATUS_FINISHED, savedBytes, timer.elapsed()));
    //qInfo("TsRecord completed after %lli ms, %lli MB for an average bitrate: %lli Kbps", timer.elapsed(), savedBytes/1000000, (savedBytes*8/1000000)/timer.elapsed());
}
