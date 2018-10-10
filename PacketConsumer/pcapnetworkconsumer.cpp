#include "pcapnetworkconsumer.h"
#include "packetparser.h"
#include "../PacketProducer/pcapproduct.h"
#include <QElapsedTimer>
#include <QtNetwork>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef linux
#include <unistd.h>
#endif

#include "igmp.h"

void PcapNetworkConsumer::start(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &PcapNetworkConsumer::run);
    connect(this, &PcapNetworkConsumer::finished, thread, &QThread::quit);

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
    if (networkHandle == nullptr) {
        qInfo("networkHandle not opened");
        QString error = QString(tr("Failed to open network adapter."));
        emit status(Status(error));
        emit finished();
        return;
    }

    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    QElapsedTimer elapsedTimer;
    QElapsedTimer statusTimer;

    Product packet = prevProvider->getProduct();
    pkt_data = (const u_char*)packet.data.data();
    header = (pcap_pkthdr*)packet.header.data();

    PacketParser parser;
    parser.parse(header, pkt_data);

    // Try to increase priority of current thread
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
#ifdef linux
    nice(-10);
#endif

    qint64 sentBytes = 0, lastSentBytes = 0;

    // Set this to 1 s less so we can send first packet on time
    std::chrono::microseconds recordedStartTime(std::chrono::seconds(header->ts.tv_sec - 1) +
                                                std::chrono::microseconds(header->ts.tv_usec));
    auto startTime = std::chrono::steady_clock::now();
    std::chrono::nanoseconds packetOffset;

    emit status(Status(Status::STATUS_STARTED));

    elapsedTimer.start();
    statusTimer.start();

    while (!stopping && packet.type == PcapProduct::NORMAL) {
        pkt_data = (const u_char*)packet.data.data();
        header = (pcap_pkthdr*)packet.header.data();

        // Do lots of stuff
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

        packetOffset = std::chrono::seconds(header->ts.tv_sec) + std::chrono::microseconds(header->ts.tv_usec) - recordedStartTime;

        if (std::chrono::steady_clock::now() < startTime + packetOffset) {
            std::this_thread::sleep_until(startTime + packetOffset);
        }

        if (!pcap_sendpacket(networkHandle, pkt_data, header->len)) {
            sentBytes += parser.data_len;
        }

        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, sentBytes, elapsedTimer.elapsed(),
                               ((sentBytes-lastSentBytes)*8*1000)/statusTimer.elapsed()));
            lastSentBytes = sentBytes;
            statusTimer.restart();
        }

        packet = prevProvider->getProduct();
    }

    const qint64 elapsed = elapsedTimer.elapsed();

    emit status(Status(Status::STATUS_FINISHED, sentBytes, elapsed));

    if (elapsed > 0) {
        qInfo("Playback completed after %lli ms, %lli KB for an average bitrate: %lli Kbps", elapsedTimer.elapsed(), sentBytes/1000, (sentBytes*8/1000000)/elapsedTimer.elapsed());
    }
    else {
        qInfo("Playback completed after 0 ms");
    }

    pcap_close(networkHandle);
}
