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
    if (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE || config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_LIVE) {
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
    PcapProduct packet = prevProvider->getProduct();
    while (!stopping && (packet.type != PcapProduct::END && packet.type != PcapProduct::STOP)) {
        packet = prevProvider->getProduct();
    }
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

    PcapProduct packet = prevProvider->getProduct();
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
