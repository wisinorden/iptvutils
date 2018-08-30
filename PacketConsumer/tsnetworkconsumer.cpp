#include "tsnetworkconsumer.h"
#include "packetparser.h"
#include "../PacketProducer/tsproduct.h"
#include <QElapsedTimer>
#include <QtNetwork>
#include <queue>
#include <thread>

#include "igmp.h"

void TsNetworkConsumer::start(QThread *thread) {
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &TsNetworkConsumer::run);
    connect(this, &TsNetworkConsumer::finished, thread, &QThread::quit);
    thread->start(QThread::TimeCriticalPriority);
}

void TsNetworkConsumer::stop() {
    stopping = true;
}

void TsNetworkConsumer::run() {
    emit started();
    qInfo("Running TsNetworkConsumer in thread: %p", QThread::currentThread());
    if (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE ||
            config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_LIVE) {
        qInfo("Running TsNetworkConsumer in Analysis mode");
        analysisMode();
    }
    else {
        qInfo("Playing from q network");
        playVBRFromQNetwork();
        //playFromQNetwork();
    }

    emit finished();
    this->thread()->quit();
    qInfo("TsNetworkConsumer finished");
}

void TsNetworkConsumer::analysisMode() {
    Product packet = prevProvider->getProduct();
    while (packet.type != Product::END && packet.type != Product::STOP) {
        packet = prevProvider->getProduct();
    }
}

/**
 * @brief Calculates the PCR of a given TS packet containing one.
 * @param t_ts_pkt 188 byte TS packet.
 * @return The PCR of the packet.
 */
qint64 TsNetworkConsumer::getPcr(QByteArray t_ts_pkt) {
    qint64 pcr_base = 0, pcr_ext = 0;
    pcr_base |= (((qint64) t_ts_pkt.data()[6] << 25) & 0x1ffffffff);
    pcr_base |= (((qint64) t_ts_pkt.data()[7] << 17) & 0x1ffffff);
    pcr_base |= (((qint64) t_ts_pkt.data()[8] << 9) & 0x1ffff);
    pcr_base |= (((qint64) t_ts_pkt.data()[9] << 1) & 0x1ff);
    pcr_base |= (t_ts_pkt.data()[10] & 0x80) >> 7;
    pcr_base *= 300;
    pcr_ext |= (t_ts_pkt.data()[10] & 0x01) << 8;
    pcr_ext |= (qint64) (t_ts_pkt.data()[11] & 0x000000ff);

    return pcr_base + pcr_ext;
}

void TsNetworkConsumer::playVBRFromQNetwork() {
    QHostAddress address = QHostAddress(config.getNetworkOutput().getHost());
    QUdpSocket* socket = new QUdpSocket(this);
    bool bound = socket->bind(config.getNetworkOutput().getDevice().getAddress());
    if (!bound) {
        qInfo("Bound error: %i", socket->error());
        Q_ASSERT(false);
    }

    bool time_point_set = false;
    bool end = false;
    QElapsedTimer timer;
    QElapsedTimer status_timer;
    std::queue<qint64> q_packet_nr;
    std::queue<long double> q_packet_rate;
    std::queue<QByteArray> q_packet;
    QByteArray dg = 0;
    qint64 packet_nr = 0;
    qint8 send_after = 7; // how many ts packets to send at a time
    qint64 packet_count = 0;
    qint64 pcr1 = 0, pcr2 = 0;
    qint64 pn1 = 0, pn2 = 0;
    qint64 sent_bytes = 0, last_status_bytes = 0;
    long double packet_rate = 0;
    Product packet;
    std::chrono::high_resolution_clock::time_point tp;
    tp = std::chrono::high_resolution_clock::now();
    status_timer.start();
    timer.start();
    while (!stopping) { // fixa så att den inte stoppar direkt när man får END
        if (!end) {
            ++packet_nr;
            packet = prevProvider->getProduct();
            q_packet.push(packet.data);
            if (((packet.data.data()[3] & 0x30) > 0x10) &&
                    (packet.data.data()[4] >= 0x01) &&
                    ((packet.data.data()[5] & 0x10) == 0x10)) {
                if (pcr1 == 0) {
                    pcr1 = getPcr(packet.data);
                    pn1 = packet_nr;
                }
                else {
                    pcr2 = getPcr(packet.data);
                    pn2 = packet_nr;
                    packet_rate = 1/(((long double) (pn2-pn1)/(pcr2-pcr1))*
                                     27000000);

                    pcr1 = pcr2;
                    pcr2 = 0;
                    pn1 = pn2;
                    q_packet_nr.push(packet_nr);
                    q_packet_rate.push(packet_rate);
                }
            }
        }

        if (!q_packet_rate.empty() && !q_packet.empty() && !stopping) {
            ++packet_count;
            std::chrono::duration<int, std::micro> datagram_interval(
                        (int) (q_packet_rate.front()*send_after*1000000));
            if (packet_count == q_packet_nr.front()) {
                q_packet_rate.pop();
                q_packet_nr.pop();
            }

            dg += q_packet.front();
            q_packet.pop();
            if ((packet_count % send_after) == 0) {
                if (!time_point_set) {
                    tp = std::chrono::high_resolution_clock::now();
                    time_point_set = true;
                }

                tp += datagram_interval;
                std::this_thread::sleep_until(tp);
                sent_bytes += socket->writeDatagram(
                            dg, address,
                            config.getNetworkOutput().getPort());
                dg = 0;
                if (status_timer.elapsed() >= 1000) {
                    emit status(Status(Status::STATUS_PERIODIC, sent_bytes,
                                       timer.elapsed(),
                                       ((sent_bytes-last_status_bytes)*8*1000)/
                                       status_timer.elapsed()));
                    qInfo("packet count: %ld", packet_count);
                    qInfo("sent: %ld, last: %ld", sent_bytes, last_status_bytes);
                    qInfo("STATUS: %d", ((sent_bytes-last_status_bytes)*8*1000)/
                          status_timer.elapsed());
                    last_status_bytes = sent_bytes;
                    status_timer.restart();
                }
            }
        }
        else {
            if (end) break;
        }

        if (packet.type == PcapProduct::END ||
                packet.type == PcapProduct::STOP) {
            end = true;
        }
    }

    emit status(Status(Status::STATUS_FINISHED, sent_bytes, timer.elapsed()));
    if (timer.elapsed() > 0) {
        qInfo("Playback completed after %lli ms, %lli KB for an average "
              "bitrate: %lli Kbps", timer.elapsed(), sent_bytes/1000,
              (sent_bytes*0.000008)/timer.elapsed());
    }
    else {
        qInfo("Playback completed after 0 ms");
    }

    socket->close();
    delete socket;
    qInfo("play from vbr func done");
}

void TsNetworkConsumer::playFromQNetwork() {
    QHostAddress address = QHostAddress(config.getNetworkOutput().getHost());
    QUdpSocket* socket = new QUdpSocket(this);
    bool bound = socket->bind(config.getNetworkOutput().getDevice().getAddress());
    if (!bound) {
        qInfo("Bound error: %i", socket->error());
        Q_ASSERT(false);
    }

    quint16 pcr_pid; // holder variable for the chosen pcr used for calculation
    quint64 packetNr = 0;
    quint8 added = 0;
    std::vector<qint64> pcr_val;
    std::vector<qint64> pkt_no;
    std::queue<QByteArray> datagrams;
    QByteArray dg;
    QElapsedTimer timer;
    QElapsedTimer statusTimer;
    qint64 sentBytes = 0, lastStatusBytes = 0;
    bool delta = false;
    bool hasLooped = false;
    Product input;
    while (!stopping) {
        input = prevProvider->getProduct();
        dg += input.data;
        if ((packetNr+1) % 7 == 0) {
            datagrams.push(dg);
            dg = 0;
        }
        if (input.type == PcapProduct::NORMAL && !hasLooped) {
            /* uncomment when pcr pid choice has been implemented
            if ((((input.data.data()[1] & 0x1f) << 8 +
                 (input.data.data()[2] & 0xff)) == pcr_pid) &&
                    ((input.data.data()[3] & 0x30) > 0x10) &&
                    (input.data.data()[4] >= 0x01) &&
                    ((input.data.data()[5] & 0x10) == 0x10))

            */

            if (((input.data.data()[3] & 0x30) > 0x10) &&
                    (input.data.data()[4] >= 0x01) &&
                    ((input.data.data()[5] & 0x10) == 0x10)) {
                // has adaption field and pcr flag is set
                pcr_val.push_back(getPcr(input.data));
                qInfo("packet number: %lld, pcr: %llx", packetNr, getPcr(input.data));
                qInfo() << getPcr(input.data);
                pkt_no.push_back(packetNr);
                ++added;
                if (added == 2) {
                    added = 0;
                    delta = true;
                }
            }
            if (delta) {
                // if two PCR values have been added, the delta PCR and packet
                // number is calculated
                qint64 val1 = 0, val2 = 0;
                val1 = pcr_val.back();
                pcr_val.pop_back();
                val2 = pcr_val.back();
                pcr_val.pop_back();
                pcr_val.push_back(val1 - val2);
                val1 = pkt_no.back();
                pkt_no.pop_back();
                val2 = pkt_no.back();
                pkt_no.pop_back();
                pkt_no.push_back(val1 - val2);
                delta = false;
            }
        }
        else if (input.type == PcapProduct::LOOP) hasLooped = true;

        ++packetNr;
        if (input.type == PcapProduct::END ||
                input.type == PcapProduct::STOP) break;
    }

    quint64 pkt_no_sum = 0, pcr_val_sum = 0;
    quint64 pkt_size = pkt_no.size(), pcr_size = pcr_val.size();
    while (!pcr_val.empty()) {
        pkt_no_sum += pkt_no.back();
        pcr_val_sum += pcr_val.back();
        pkt_no.pop_back();
        pcr_val.pop_back();
    }

    long double packetRate = 1/((long double) (pkt_no_sum/pkt_size)/
                           (pcr_val_sum/pcr_size)*
                           27000000);
    std::chrono::high_resolution_clock::time_point tp;
    tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int, std::micro> datagram_interval(
                (int) (packetRate*7*1000000));
    statusTimer.start();
    timer.start();
    while (!datagrams.empty() && !stopping) {
        tp += datagram_interval;
        std::this_thread::sleep_until(tp);
        sentBytes += socket->writeDatagram(datagrams.front(), address,
                              config.getNetworkOutput().getPort());
        datagrams.pop();
        if (statusTimer.elapsed() >= 1000) {
            emit status(Status(Status::STATUS_PERIODIC, sentBytes,
                               timer.elapsed(),
                               ((sentBytes-lastStatusBytes)*8*1000)/
                               statusTimer.elapsed()));
            lastStatusBytes = sentBytes;
            statusTimer.restart();
        }
    }

    emit status(Status(Status::STATUS_FINISHED, sentBytes, timer.elapsed()));
    if (timer.elapsed() > 0) {
        qInfo("Playback completed after %lli ms, %lli KB for an average "
              "bitrate: %lli Kbps", timer.elapsed(), sentBytes/1000,
              (sentBytes*0.000008)/timer.elapsed());
    }
    else {
        qInfo("Playback completed after 0 ms");
    }

    socket->close();
    delete socket;
}
