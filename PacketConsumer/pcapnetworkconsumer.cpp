#include "pcapnetworkconsumer.h"
#include "packetparser.h"
#include "../PacketProducer/pcapproduct.h"
#include <QElapsedTimer>
#include <QtNetwork>

#include "igmp.h"

void PcapNetworkConsumer::start(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &PcapNetworkConsumer::run);
    connect(this, &PcapNetworkConsumer::finished, thread, &QThread::quit);
    //connect(this, &PcapNetworkConsumer::finished, &QThread::deleteLater);
    //connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start(QThread::TimeCriticalPriority);
}

void PcapNetworkConsumer::stop() {
    stopping = true;
}

void PcapNetworkConsumer::run() {
    emit started();
    qInfo("Running PcapNetworkconsumer in thread: %p", QThread::currentThread());
    if (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE ||
            config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_LIVE) {
        qInfo("Running PcapNetworkconsumer in Analysis mode");
        analysisMode();
    }
    else if (config.getInputType() == WorkerConfiguration::FILE && config.getFileInput().getLoopStyle() == FileInputConfiguration::LOOP_SIMPLE) {
        do {
            qInfo("playing loop");
            playFromPcapNetwork();
        }
        while(!stopping);
    }
    else {
        playFromPcapNetwork();
        //playFromQNetwork();
    }
    qInfo("PcapNetworkConsumer finished");
    emit finished();
    this->thread()->quit();
}

void PcapNetworkConsumer::analysisMode() {
    Product packet = prevProvider->getProduct();
    while (!stopping && (packet.type != PcapProduct::END && packet.type != PcapProduct::STOP)) {
        packet = prevProvider->getProduct();
    }
}

void PcapNetworkConsumer::playFromQNetwork() {
    QHostAddress address = QHostAddress(config.getNetworkOutput().getHost());
    QString qs = address.toString();
    QUdpSocket* socket = new QUdpSocket(this);
    qInfo() << socket->state();
//    connect(
//        socket, &QUdpSocket::bytesWritten,
//        [=]( const qint64 &bytes ) { if (bytes < 1000) qInfo("written %lli", bytes ); }
//    );
    bool bound = socket->bind(config.getNetworkOutput().getDevice().getAddress());
    if (!bound) {
        qInfo("BoundError: %i", socket->error());
        Q_ASSERT(false);
    }

    socket->setMulticastInterface(config.getNetworkOutput().getDevice().getQInterface());

    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    QNetworkInterface multicastIf = socket->multicastInterface();
    Q_ASSERT(multicastIf.isValid());
    Q_ASSERT(multicastIf.index() == config.getNetworkOutput().getDevice().getQInterface().index());

//    socket->connectToHost(address, config.getNetworkOutput().getPort());
//    if (socket->waitForConnected(1000)) {
//        qDebug("Connected!");
//    }

    Q_ASSERT(socket->isValid());

    QElapsedTimer timer;
    timer.start();
    QElapsedTimer statusTimer;

    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    Product packet = prevProvider->getProduct();
    pkt_data = (const u_char*)packet.data.data();
    header = (pcap_pkthdr*)packet.header.data();

    PacketParser parser;
    parser.parse(header, pkt_data);

    qint64 sinceStart;
    const qint64 usecStart = header->ts.tv_sec * 1000000 +  header->ts.tv_usec;
    statusTimer.start();
    qint64 sentBytes = 0;
    qint64 packetNumber = 1;
    qint64 lastStatusBytes = 0;
    qint64 lastSentPackets = 0;

    QElapsedTimer sleepTimer, sendTimer, getTimer;
    qint64 statusSleep = 0, statusSend = 0, statusGet = 0;

    emit status(Status(Status::STATUS_STARTED));


    while (!stopping && packet.type == PcapProduct::NORMAL) {
        pkt_data = (const u_char*)packet.data.data();
        header = (pcap_pkthdr*)packet.header.data();

        parser.parse(header, pkt_data);

        sleepTimer.start();
        sinceStart = timer.nsecsElapsed();
        if ((header->ts.tv_sec*1000000 + header->ts.tv_usec) - (usecStart+(sinceStart/1000l)) > 0) {
            QThread::usleep((header->ts.tv_sec*1000000 + header->ts.tv_usec) - (usecStart+(sinceStart/1000l)));
        }
        statusSleep += sleepTimer.nsecsElapsed();


        sendTimer.start();
        qint64 bytes = socket->writeDatagram((const char*)parser.rp_data, parser.rp_data_len, address, config.getNetworkOutput().getPort());
        //qint64 bytes = socket->write((const char*) parser.rp_data, parser.rp_data_len);
        //qint64 bytes = parser.rp_data_len;

        socket->flush();
        //Q_ASSERT(bytes > 0);
        if (bytes < 1000) {
            qWarning("Qnet: Error writing: wrote %lli bytes, error %i: %s", bytes, socket->error(), qPrintable(socket->errorString()));
        }

        sentBytes += bytes;
        statusSend += sendTimer.nsecsElapsed();
        //sentBytes += parser.data_len;
        lastSentPackets++;
        packetNumber++;

        if (socket->hasPendingDatagrams()) {
            qInfo("Has pending");
        }

        // Every one second, send out status information
        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, sentBytes, timer.elapsed(), ((sentBytes-lastStatusBytes)*8*1000)/statusTimer.elapsed()));
            qInfo("Status: %lli Kbytes sent, Bitrate: %lli kbps, %lli packets",
                  sentBytes/1000,
                  ((sentBytes-lastStatusBytes)*8)/statusTimer.elapsed(),
                  lastSentPackets);
            qInfo("sleep: %lli %%, send: %lli %%, getting next: %lli %%",
                  (statusSleep/10000)/statusTimer.elapsed(),
                  (statusSend/10000)/statusTimer.elapsed(),
                  (statusGet/10000)/statusTimer.elapsed());
            lastStatusBytes = sentBytes;
            lastSentPackets = 0;
            statusTimer.restart();
            statusSleep = statusSend = statusGet = 0;
        }

        getTimer.start();
        packet = prevProvider->getProduct();
        statusGet += getTimer.nsecsElapsed();
    }

    emit status(Status(Status::STATUS_FINISHED, sentBytes, timer.elapsed()));

    if (timer.elapsed() > 0) {
        qInfo("Playback completed after %lli ms, %lli KB for an average bitrate: %lli Kbps", timer.elapsed(), sentBytes/1000, (sentBytes*8/1000000)/timer.elapsed());
    }
    else {
        qInfo("Playback completed after 0 ms");
    }

    socket->close();
    delete socket;
}

void PcapNetworkConsumer::playFromPcapNetwork() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *networkHandle = pcap_open_live(config.getNetworkOutput().getDevice().getId().toLocal8Bit().constData(),       // name of the device
                        65536,		// portion of the packet to capture.
                                    // 65536 grants that the whole packet will be captured on all the MACs.
                        1,			// promiscuous mode (nonzero means promiscuous)
                        1000,		// read timeout
                        errbuf     	// error buffer
                       );

    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    QElapsedTimer timer;
    timer.start();
    QElapsedTimer statusTimer;

    Product packet = prevProvider->getProduct();
    pkt_data = (const u_char*)packet.data.data();
    header = (pcap_pkthdr*)packet.header.data();

    PacketParser parser;
    parser.parse(header, pkt_data);

    qint64 sinceStart;
    const qint64 usecStart = header->ts.tv_sec * 1000000 +  header->ts.tv_usec;
    statusTimer.start();
    qint64 sentBytes = 0;
    qint64 packetNumber = 1;
    qint64 lastStatusBytes = 0;
    qint64 lastSentPackets = 0;

    QElapsedTimer sleepTimer, sendTimer, getTimer;
    qint64 statusSleep = 0, statusSend = 0, statusGet = 0;

    emit status(Status(Status::STATUS_STARTED));

    while (!stopping && packet.type == PcapProduct::NORMAL) {
        pkt_data = (const u_char*)packet.data.data();
        header = (pcap_pkthdr*)packet.header.data();

        parser.parse(header, pkt_data);
        if (config.getNetworkOutput().rewriteSrcHost()) {
            parser.changeSourceIpAndMac(
                        config.getNetworkOutput().getDevice().getAddress().toString(),
                        config.getNetworkOutput().getPort(),
                        config.getNetworkOutput().getDevice().getMac());
        }
        if (config.getNetworkOutput().rewriteDstHost()) {
            parser.changeMulticastDestination(config.getNetworkOutput().getHost());
        }
        if (config.getNetworkOutput().rewriteDstPort()) {
            parser.changePort(config.getNetworkOutput().getPort());
        }

        sleepTimer.start();
        sinceStart = timer.nsecsElapsed();
        if ((header->ts.tv_sec*1000000 + header->ts.tv_usec) - (usecStart+(sinceStart/1000l)) > 0) {
            QThread::usleep((header->ts.tv_sec*1000000 + header->ts.tv_usec) - (usecStart+(sinceStart/1000l)));
        }
        statusSleep += sleepTimer.nsecsElapsed();


        sendTimer.start();
        if (!pcap_sendpacket(networkHandle, pkt_data, header->len)) {
            sentBytes += parser.data_len;
        }
        statusSend += sendTimer.nsecsElapsed();
        //sentBytes += parser.data_len;
        lastSentPackets++;
        packetNumber++;

        // Every one second, send out status information
        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, sentBytes, timer.elapsed(), ((sentBytes-lastStatusBytes)*8*1000)/statusTimer.elapsed()));
//            qInfo("Status not yet set up: %lli Kbytes sent, Bitrate: %lli kbps, %lli packets", sentBytes/1000, ((sentBytes-lastStatusBytes)*8/1000000)/statusTimer.elapsed(), lastSentPackets);
            qInfo("sleep: %lli %%, send: %lli %%, getting next: %lli %%",
                  (statusSleep/10000)/statusTimer.elapsed(),
                  (statusSend/10000)/statusTimer.elapsed(),
                  (statusGet/10000)/statusTimer.elapsed());
            lastStatusBytes = sentBytes;
            lastSentPackets = 0;
            statusTimer.restart();
            statusSleep = statusSend = statusGet = 0;
        }

        getTimer.start();
        packet = prevProvider->getProduct();
        statusGet += getTimer.nsecsElapsed();
    }

    emit status(Status(Status::STATUS_FINISHED, sentBytes, timer.elapsed()));

    if (timer.elapsed() > 0) {
        qInfo("Playback completed after %lli ms, %lli KB for an average bitrate: %lli Kbps", timer.elapsed(), sentBytes/1000, (sentBytes*8/1000000)/timer.elapsed());
    }
    else {
        qInfo("Playback completed after 0 ms");
    }

    pcap_close(networkHandle);
}
