#include "pcapbufferedproducer.h"
#include "packetparser.h"
#include "igmp.h"
#include <QElapsedTimer>

void PcapBufferedProducer::run() {
    emit started();
    qInfo("Running PcapBufferedProducer in thread: %p", QThread::currentThread());

    if (config.getInputType() == WorkerConfiguration::FILE) {
        bufferFromFile();
    }
    else if (config.getInputType() == WorkerConfiguration::NETWORK) {
        IGMP::joinMulticastGroup(config.getNetworkInput().getHost());
        bufferFromNetwork();
        IGMP::leaveMulticastGroup(config.getNetworkInput().getHost());
    }
    else {
        qCritical("PcapBufferedProducer: Unknown input type");
    }
    qInfo("PcapBufferedProducer finished");
    emit finished();
    this->thread()->quit();
}

void PcapBufferedProducer::stop() {
    stopping = true;
    buffer.stop();
}

void PcapBufferedProducer::init(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started,  this, &PcapBufferedProducer::run);
    connect(this, &PcapBufferedProducer::finished, thread, &QThread::quit);
    //connect(this, &PcapBufferedProducer::finished, this, &PcapBufferedProducer::deleteLater);
    //connect(thread, &QThread::finished, thread, &QThread::deleteLater);
}

void PcapBufferedProducer::bufferFromFile() {
    qInfo("Buffering pcap from file, buffer size: %lu", buffer.max_size());
    char errbuf[PCAP_ERRBUF_SIZE];

    // Filter
    u_int netmask;
    struct bpf_program fcode;

    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    do {
        pcap_t *buffer_handler = pcap_open_offline(
                    config.getFileInput().getFilename().toLocal8Bit().constData(),
                    errbuf);

        if (buffer_handler == NULL) {
            QString error = QString(tr("Failed to open file\n%1")).arg(config.getFileInput().getFilename());
            emit status(Status(error));
            qInfo("PcapBufferedProducer: could not open file %s",
                   qPrintable(config.getFileInput().getFilename()));
            return;
        }

        /*
         * FILTER
         */

        netmask = 0xffffff;

        //compile the filter
        if (pcap_compile(buffer_handler, &fcode, config.getFileInput().getFilter().toLocal8Bit().constData(), 1, netmask) < 0)
        {
            qCritical("\nUnable to compile the packet filter \"%s\". Check the syntax.\n", config.getFileInput().getFilter().toLocal8Bit().constData());
            QString error = QString(tr("Invalid filter."));
            emit status(Status(error));
            return;
        }

        //set the filter
        if (pcap_setfilter(buffer_handler, &fcode) < 0)
        {
            qCritical("\nError setting the filter.\n");
            QString error = QString(tr("Error setting the filter."));
            emit status(Status(error));
            return;
        }

        /*
         * END OF FILTER
         */

        while(!stopping && pcap_next_ex(buffer_handler, &header, &pkt_data) > 0) {
            buffer.push(PcapProduct(pkt_data, (const char*)header, header->len));
            PacketParser parser;
            parser.parse(header, pkt_data);
            streams[StreamId::calcId(parser.ih->daddr, parser.dport)].bytes += header->len;
            //qInfo("id %llx", StreamId::calcId(parser.ih->daddr, parser.dport));
        }

        // if not force stopped
        if (!stopping) {
            if (config.getFileInput().getLoopStyle() != FileInputConfiguration::LOOP_NONE &&
                    config.getWorkerMode() == WorkerConfiguration::NORMAL_MODE) {
                buffer.push(PcapProduct(PcapProduct::LOOP));
            }
            else {
                buffer.push(PcapProduct(PcapProduct::END));
            }
        }

        pcap_close(buffer_handler);
    }
    while ( !stopping &&
            config.getWorkerMode() == WorkerConfiguration::NORMAL_MODE &&
            config.getFileInput().getLoopStyle() != FileInputConfiguration::LOOP_NONE );
    // Prevent loops when analyzing

    emit workerStatus(WorkerStatus(WorkerStatus::STATUS_FINISHED, streams));
}

void PcapBufferedProducer::bufferFromNetwork() {
    char errbuf[PCAP_ERRBUF_SIZE];

    // Filter
    u_int netmask;
    struct bpf_program fcode;

    networkHandle = pcap_open_live(config.getNetworkInput().getDevice().getId().toLocal8Bit().constData(),       // name of the device
                        65536,		// portion of the packet to capture.
                                    // 65536 grants that the whole packet will be captured on all the MACs.
                        1,			// promiscuous mode (nonzero means promiscuous)
                        1001,       // read timeout
                        errbuf     	// error buffer
                       );

    if (networkHandle == NULL) {
        qInfo("networkHandle not opened");
        QString error = QString(tr("Failed to open network adapter."));
        emit status(Status(error));
        return;
    }

    if (pcap_datalink(networkHandle) != DLT_EN10MB) {
        qInfo("Device doesn't provide Ethernet headers - not supported\n");
        QString error = QString(tr("Network adapter doesn't provide Ethernet headers\nnot supported"));
        emit status(Status(error));
        return;
    }

    /*
     * FILTER
     */

    netmask = 0xffffff;

    //compile the filter
    if (pcap_compile(networkHandle, &fcode, config.getNetworkInput().getFilter().toLocal8Bit().constData(), 1, netmask) < 0)
    {
        qCritical("\nUnable to compile the packet filter \"%s\". Check the syntax.\n", config.getNetworkInput().getFilter().toLocal8Bit().constData());
        QString error = QString(tr("Invalid filter."));
        emit status(Status(error));
        return;
    }

    //set the filter
    if (pcap_setfilter(networkHandle, &fcode) < 0)
    {
        qCritical("\nError setting the filter.\n");
        QString error = QString(tr("Error setting the filter."));
        emit status(Status(error));
        return;
    }

    /*
     * END OF FILTER
     */

    const u_char *pkt_data;
    struct pcap_pkthdr *header;

    PacketParser parser;
    qint64 bytes = 0;
    qint64 lastStatusBytes = 0;

    QElapsedTimer timer;
    timer.start();
    QElapsedTimer statusTimer;
    statusTimer.start();

    emit status(Status(Status::STATUS_STARTED));

    int pcapStatus = 5; // some value that does not otherwise appear as status

    while(!stopping && (pcapStatus = pcap_next_ex(networkHandle, &header, &pkt_data)) >= 0) {
        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, bytes, timer.elapsed(), ((bytes-lastStatusBytes)*8*1000)/statusTimer.elapsed()));
            lastStatusBytes = bytes;
            statusTimer.restart();
        }
        // pcap timeout, no data is contained here
        if (pcapStatus == 0) {
            continue;
        }
        parser.parse(header, pkt_data);
        bytes += parser.data_len;
        buffer.push(PcapProduct(pkt_data, (const char*)header, header->len));

    }

    if (pcapStatus == -1) {
        QString statusName = "PCAP ERROR: " + QString(pcap_geterr(networkHandle));
        qInfo("PcapBufferedProducer: pcap_next_ex returned with status: %s", qPrintable(statusName));
        emit status(Status(statusName));
    }

    emit status(Status(Status::STATUS_FINISHED, bytes, timer.elapsed()));

    // End of stream packet
    buffer.push(PcapProduct());

    pcap_close(networkHandle);
}

PcapProduct PcapBufferedProducer::getProduct() {
    return buffer.pop();
}
