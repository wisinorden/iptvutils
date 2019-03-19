#include "pcapbufferedproducer.h"
#include "packetparser.h"
#include "igmp.h"
#include <QElapsedTimer>
#include <QSocketNotifier>

void PcapBufferedProducer::init(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, SIGNAL(started()),  this, SLOT(setup()));
    connect(this, SIGNAL(finished()), thread, SLOT(quit()));
    connect(this, SIGNAL(_internalStopSignal()), this, SLOT(_internalStop()));
}

PcapProduct PcapBufferedProducer::getProduct() {
    return buffer.pop();
}

void PcapBufferedProducer::stop() {
    stopping = true;
    buffer.stop();
    if (config.getInputType() == WorkerConfiguration::NETWORK) {
#ifndef Q_OS_WIN
        emit _internalStopSignal();
#endif
    }
}

void PcapBufferedProducer::setup() {
    qInfo("Running PcapBufferedProducer in thread: %p", QThread::currentThread());

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
    else if (config.getInputType() == WorkerConfiguration::NETWORK) {
        if (bufferFromNetworkSetup() == 0) {
            // Enter run loop (blocking on some platforms, event loop on others
            bufferFromNetworkRun();
        }
        else {
            bufferFromNetworkTeardown();
        }
    }
    else {
        qCritical("PcapBufferedProducer: Unknown input type");
    }
}

#ifndef Q_OS_WIN

void PcapBufferedProducer::_internalStop() {
    bufferFromNetworkTeardown();
}

/**
 * @brief Event handler for "packets available" on libpcap socket
 * @param socket Not used
 */
void PcapBufferedProducer::networkSocketActivated(int socket)
{
    Q_UNUSED (socket)

    networkSocketReadout();
    networkSocketTimer->start(1000);
}

/**
 * @brief Timer handler for dealing with selects (internal to libpcap)
 * not timing out correctly, happens on some platforms
 */
void PcapBufferedProducer::networkSocketTimeout()
{
    networkSocketReadout();
    networkSocketTimer->start(1000);
}

/**
 * @brief Timer handler for status update from network PCAP
 */
void PcapBufferedProducer::networkSocketStatusUpdate()
{
    qint64 timeNow = elapsedTimer.elapsed();
    if(timeNow - statusLastTime > 999){
        emit status(Status(Status::STATUS_PERIODIC, bytes, timeNow, ((bytes-statusLastBytes)*8*1000)/(timeNow-statusLastTime)));
        statusLastBytes = bytes;
        statusLastTime = timeNow;
    }
}

#endif /* ifndef Q_OS_WIN */

int PcapBufferedProducer::bufferFromFileSetup() {
    qInfo("Buffering pcap from file, buffer size: %lu", buffer.max_size());

    // Inform that we are starting
    emit started();

    // Since PCAP file needs to be reopened for every loop, there is not much to do here...

    return 0;
}

void PcapBufferedProducer::bufferFromFileRun()
{
    u_int netmask;
    struct bpf_program fcode;
    const u_char *pkt_data;
    struct pcap_pkthdr *header;
    PacketParser parser;
    int ret;

    while (!stopping ) {
        pcapHandle = pcap_open_offline(
                    config.getFileInput().getFilename().toLocal8Bit().constData(),
                    pcap_errbuf);

        if (pcapHandle == NULL) {
            QString error = QString(tr("Failed to open file\n%1")).arg(config.getFileInput().getFilename());
            emit status(Status(error));
            qInfo("PcapBufferedProducer: could not open file %s",
                   qPrintable(config.getFileInput().getFilename()));
            break;
        }

        /*
         * FILTER
         */

        netmask = 0xffffff;

        //compile the filter
        if (pcap_compile(pcapHandle, &fcode, config.getFileInput().getFilter().toLocal8Bit().constData(), 1, netmask) < 0)
        {
            qCritical("\nUnable to compile the packet filter \"%s\". Check the syntax.\n", config.getFileInput().getFilter().toLocal8Bit().constData());
            QString error = QString(tr("Invalid filter."));
            emit status(Status(error));
            pcap_close(pcapHandle);
            pcapHandle = NULL;
            break;
        }

        //set the filter
        if (pcap_setfilter(pcapHandle, &fcode) < 0)
        {
            qCritical("\nError setting the filter.\n");
            QString error = QString(tr("Error setting the filter."));
            emit status(Status(error));
            pcap_close(pcapHandle);
            pcapHandle = NULL;
            break;
        }

        while (!stopping && (ret = pcap_next_ex(pcapHandle, &header, &pkt_data)) > 0) {
            buffer.push(PcapProduct(pkt_data, (const char*)header, header->len));
            parser.parse(header, pkt_data);
            streams[StreamId::calcId(parser.ih->daddr, parser.dport)].bytes += header->len;
        }

        if (ret == -2) {
            if (config.getFileInput().getLoopStyle() != FileInputConfiguration::LOOP_NONE &&
                config.getWorkerMode() == WorkerConfiguration::NORMAL_MODE) {
                // EOF; We should loop (e.g. restart from beginning of file)
                pcap_close(pcapHandle);
                pcapHandle = NULL;
                buffer.push(PcapProduct(PcapProduct::LOOP));
                continue;
            }
        }
        else if (ret < 0) {
            qCritical("\npcap_next_ex returned error=%d.\n", ret);
            QString error = QString(tr("PCAP read failed."));
            emit status(Status(error));
        }

        // If we get this far, abort from loop
        break;
    }

    if (pcapHandle != NULL) {
        pcap_close(pcapHandle);
        pcapHandle = NULL;
    }

    buffer.push(PcapProduct(PcapProduct::END));

    emit workerStatus(WorkerStatus(WorkerStatus::STATUS_FINISHED, streams));

    bufferFromFileTeardown();
}

void PcapBufferedProducer::bufferFromFileTeardown() {
    emit finished();
    QThread::currentThread()->quit();
}

int PcapBufferedProducer::bufferFromNetworkSetup() {
    qInfo("Buffering pcap from network");

    // Filter
    u_int netmask;
    struct bpf_program fcode;

    // Inform that we are starting
    emit started();

    // Join multicast address
    IGMP::joinMulticastGroup(config.getNetworkInput().getHost(), config.getNetworkInput().getDevice().getQInterface());

    pcapHandle = pcap_open_live(config.getNetworkInput().getDevice().getId().toLocal8Bit().constData(),       // name of the device
                        65536,		// portion of the packet to capture.
                                    // 65536 grants that the whole packet will be captured on all the MACs.
                        1,			// promiscuous mode (nonzero means promiscuous)
                        500,        // read timeout
                        pcap_errbuf // error buffer
                       );

    if (pcapHandle == NULL) {
        qInfo("networkHandle not opened");
        QString error = QString(tr("Failed to open network adapter."));
        emit status(Status(error));
        emit status(Status(pcap_errbuf));
        emit finished();
        return -1;
    }

    if (pcap_datalink(pcapHandle) != DLT_EN10MB) {
        qInfo("Device doesn't provide Ethernet headers - not supported\n");
        QString error = QString(tr("Network adapter doesn't provide Ethernet headers\nnot supported"));
        emit status(Status(error));

        pcap_close(pcapHandle);
        pcapHandle = NULL;
        emit finished();

        return -1;
    }

    /*
     * FILTER
     */

    netmask = 0xffffff;

    //compile the filter
    if (pcap_compile(pcapHandle, &fcode, config.getNetworkInput().getFilter().toLocal8Bit().constData(), 1, netmask) < 0)
    {
        qCritical("\nUnable to compile the packet filter \"%s\". Check the syntax.\n", config.getNetworkInput().getFilter().toLocal8Bit().constData());
        QString error = QString(tr("Invalid filter."));
        emit status(Status(error));

        pcap_close(pcapHandle);
        pcapHandle = NULL;
        emit finished();

        return -1;
    }

    //set the filter
    if (pcap_setfilter(pcapHandle, &fcode) < 0)
    {
        qCritical("\nError setting the filter.\n");
        QString error = QString(tr("Error setting the filter."));
        emit status(Status(error));

        pcap_close(pcapHandle);
        pcapHandle = NULL;
        emit finished();

        return -1;
    }

    /*
     * END OF FILTER
     */

    emit status(Status(Status::STATUS_STARTED));

    // Start elapsed timer (before any potential events below)
    elapsedTimer.start();

#ifndef Q_OS_WIN
    // Setup no data timer
    networkSocketTimer = new QTimer();
    connect(networkSocketTimer, SIGNAL(timeout()), this, SLOT(networkSocketTimeout()));
    networkSocketTimer->setSingleShot(true);
    networkSocketTimer->start(1000);

    // Setup PCAP socket in non-blocking mode
    pcap_setnonblock(pcapHandle, 1, pcap_errbuf);
    int pcapFd = pcap_get_selectable_fd(pcapHandle);
    networkSocketNotifier = new QSocketNotifier(pcapFd, QSocketNotifier::Read, this);
    connect(networkSocketNotifier, SIGNAL(activated(int)), this, SLOT(networkSocketActivated(int)));

#endif /* ifndef Q_OS_WIN */

    return 0;
}

void PcapBufferedProducer::bufferFromNetworkRun() {
#ifdef Q_OS_WIN
    const u_char *pkt_data;
    struct pcap_pkthdr *header;
    PacketParser parser;
    QElapsedTimer statusTimer;
    statusTimer.start();
    int pcapStatus = 5; // some value that does not otherwise appear as status

    while(!stopping && (pcapStatus = pcap_next_ex(pcapHandle, &header, &pkt_data)) >= 0) {
        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, bytes, elapsedTimer.elapsed(), ((bytes-statusLastBytes)*8*1000)/statusTimer.elapsed()));
            statusLastBytes = bytes;
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
        QString statusName = "PCAP ERROR: " + QString(pcap_geterr(pcapHandle));
        qInfo("PcapBufferedProducer: pcap_next_ex returned with status: %s", qPrintable(statusName));
        emit status(Status(statusName));
    }

    bufferFromNetworkTeardown();
#else /* ifdef Q_OS_WIN */
    // We have an event loop setup in bufferFromNetworkSetup(), lets just silently return
#endif /* ifdef Q_OS_WIN */
}

void PcapBufferedProducer::bufferFromNetworkTeardown() {
    IGMP::leaveMulticastGroup(config.getNetworkInput().getHost(), config.getNetworkInput().getDevice().getQInterface());

    if (pcapHandle != NULL) {
#ifndef Q_OS_WIN
        disconnect(networkSocketNotifier, SIGNAL(activated(int)), this, SLOT(networkSocketActivated(int)));
        delete networkSocketNotifier;
        disconnect(networkSocketTimer, SIGNAL(timeout()), this, SLOT(networkSocketTimeout()));
        networkSocketTimer->stop();
        delete networkSocketTimer;

#endif

        emit status(Status(Status::STATUS_FINISHED, bytes, elapsedTimer.elapsed()));

        // End of stream packet
        buffer.push(PcapProduct());

        pcap_close(pcapHandle);

        pcapHandle = NULL;

        emit finished();

        QThread::currentThread()->quit();
    }
}

void PcapBufferedProducer::networkSocketReadout() {
    const u_char *pkt_data;
    struct pcap_pkthdr *header;
    PacketParser parser;
    int pcapStatus = 5; // some value that does not otherwise appear as status

    while(!stopping && (pcapStatus = pcap_next_ex(pcapHandle, &header, &pkt_data)) > 0) {
        parser.parse(header, pkt_data);
        bytes += parser.data_len;
        buffer.push(PcapProduct(pkt_data, (const char*)header, header->len));
    }

    if (pcapStatus == -1) {
        QString statusName = "PCAP ERROR: " + QString(pcap_geterr(pcapHandle));
        qInfo("PcapBufferedProducer: pcap_next_ex returned with status: %s", qPrintable(statusName));
        emit status(Status(statusName));
    } else {
        networkSocketStatusUpdate();


    }
}
