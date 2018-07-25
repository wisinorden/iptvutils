#include "pcapfileconsumer.h"
#include "packetparser.h"

#include <QElapsedTimer>

void PcapFileConsumer::start(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &PcapFileConsumer::run);
    thread->start();
}

void PcapFileConsumer::stop() {
    stopping = true;
}

void PcapFileConsumer::run() {
    emit started();
    qInfo("starting PcapFileConsumer");
    switch(config.getWorkerMode()) {
        case WorkerConfiguration::ANALYSIS_MODE_OFFLINE:
        case WorkerConfiguration::ANALYSIS_MODE_LIVE:
            analysisMode();
            break;
        case WorkerConfiguration::NORMAL_MODE:
            saveToFile();
            break;
        default:
            qFatal("PcapFileConsumer: Unknown WorkerMode");
    }
    qInfo("PcapFileConsumer finished");
    emit finished();
    this->thread()->quit();
}

void PcapFileConsumer::analysisMode() {
    qInfo("Running PcapFileConsumer in AnalysisMode");
    Product packet = prevProvider->getProduct();
    while (!stopping && packet.size > 0) {
        packet = prevProvider->getProduct();
    }
}

void PcapFileConsumer::saveToFile() {
    printf("pcap save to file\n");
    pcap_dumper_t *dumpfile;
    pcap_t *deadHandle = pcap_open_dead(DLT_EN10MB, 65536);

    dumpfile = pcap_dump_open(deadHandle, config.getFileOutput().getFilename().toLocal8Bit().constData());
    if(dumpfile==NULL)
    {
        qCritical("\nError opening output file\n");
        QString error = QString(tr("Error opening output file."));
        emit status(Status(error));
        return;
    }

    Product packet;
    QElapsedTimer timer;
    timer.start();
    QElapsedTimer statusTimer;
    statusTimer.start();

    PacketParser parser;

    packet = prevProvider->getProduct();
    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    qint64 bytes = 0;
    qint64 lastStatusBytes = 0;

    // Indicate recording has started
    emit status(Status::STATUS_STARTED);

    /* dump network data to file */
    while (!stopping && packet.type == PcapProduct::NORMAL) {
        pkt_data = (const u_char*)packet.data.data();
        header = (pcap_pkthdr*)packet.header.data();
        parser.parse(header, pkt_data);
        bytes += parser.data_len;

        pcap_dump((u_char*)dumpfile, header, pkt_data);

        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, bytes, timer.elapsed(), ((bytes-lastStatusBytes)*8*1000)/statusTimer.elapsed()));
            lastStatusBytes = bytes;
            statusTimer.restart();
        }

        packet = prevProvider->getProduct();
    }

    pcap_dump_close(dumpfile);
}
