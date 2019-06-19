#include "analyzerpcapmiddleware.h"

#include "packetparser.h"
#include "tsparser.h"
#include "Analyzer/tsanalyzer.h"
#include "Status/streamid.h"

#include <QElapsedTimer>

#include <iostream>


void AnalyzerPcapMiddleware::init() {
    this->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::started, this, &AnalyzerPcapMiddleware::run);
    connect(this, &AnalyzerPcapMiddleware::finished, &runnerThread, &QThread::quit);
}

void AnalyzerPcapMiddleware::start() {
    runnerThread.start();
}

void AnalyzerPcapMiddleware::stop() {
    stopping = true;
    buffer.stop();
}

void AnalyzerPcapMiddleware::run() {
    emit started();
    qInfo("starting AnalyzerPcapMiddleware");
    bufferProducts();
    qInfo("AnalyzerPcapMiddleware finished");
    emit finished();
    this->thread()->quit();
}

// Buffers packets
void AnalyzerPcapMiddleware::bufferProducts() {
    QElapsedTimer statusTimer;
    statusTimer.start();
    packetNumber = 0;

    qint64 startTime = -1;
    qint64 endTime = -1;

    // for bitrate calculations
    qint64 lastDuration = 0;
    qint64 lastSecondBytes = 0;

    qint64 bitrate = 0;
    qint64 bytes = 0;
    quint8 tsPerIp = 0;
    AnalyzerStatus::Protocol proto = AnalyzerStatus::UNKNOWN;

    bool hasLooped = false;

    PcapProduct input;

    while (!stopping) {
        input = prevProvider->getProduct();
        if (input.type == PcapProduct::NORMAL && !hasLooped) {
            PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
            bytes += parser.data_len;

            if (startTime == -1) {
                startTime = ((pcap_pkthdr*)input.header.data())->ts.tv_sec * 1000; // convert to ms
                startTime += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000; // convert to ms
            }
            else {
                endTime = ((pcap_pkthdr*)input.header.data())->ts.tv_sec*1000;
                endTime += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000; // convert to ms
                duration = endTime - startTime;
                if (lastDuration == 0)
                    lastDuration = duration;

                // 1 second of trafic, calculate bitrate for that second

                if (duration - lastDuration >= 200) {
                    bitrate = (bytes - lastSecondBytes)*8*1000/(duration - lastDuration);
                    lastSecondBytes = bytes;
                    lastDuration = duration;
                }
            }

            // sanity check, prevents analyzation of packets that do not contain ts-packets
            if (parser.data_len % 188 == 0 && parser.ih->proto == 17 && parser.data_len != 0) {


                tsPerIp = parser.data_len/188;

                // For now, assume the only existing protocols


                if (parser.rp_len > 0)
                    proto = AnalyzerStatus::RTP;
                else
                    proto = AnalyzerStatus::UDP;


                quint64 streamId = StreamId::calcId(parser.ih->daddr, parser.dport);
                StreamInfo &stream = streams[streamId];

                // For now, assume the only existing protocols are RTP, UDP
                if (parser.rp_len > 0)
                    stream.protocol = StreamInfo::NetworkProtocol::RTP;
                else
                    stream.protocol = StreamInfo::NetworkProtocol::UDP;

                if (stream.pidMap.contains(0x1FFF))
                    stream.bitrateMode = StreamInfo::BitrateMode::CBR;
                else
                    stream.bitrateMode = StreamInfo::BitrateMode::VBR;

                stream.id = StreamId(streamId);
                stream.bytes += parser.data_len;
                stream.tsPerIp = tsPerIp;
                stream.currentTime = duration;

                tsAnalyzer.setStream(&stream.tsErrors, &stream.pidMap);

                // Analyze every TsPacket
                for (quint8 i = 0; i < tsPerIp; i++) {
                    tsParser.parse((quint8*)(parser.data+i*188));


                    if(tsAnalyzer.validSyncByte()){

                        tsAnalyzer.ccErrorDetect();

                        // save information about this packet
                        stream.pidMap[tsParser.hdr.PID].cc = tsParser.hdr.CC;
                        stream.pidMap[tsParser.hdr.PID].scramble = tsParser.hdr.TSC;
                        stream.pidMap[tsParser.hdr.PID].lastArrived = duration;
                    }
                    else {


                    }
                }

                // case 1: sanity check true
                buffer.push(input);
                packetNumber++;


            }
            else {

                // If the IP - packet contains anything thats not TS this activates.
                qInfo("Detected packet that is not TS");
                continue;
            }

        }
        else if (input.type == PcapProduct::LOOP) {
            hasLooped = true;

            buffer.push(input);
            packetNumber++;

        }


        if (statusTimer.elapsed() >= 1000) {
            emit status(AnalyzerStatus(Status::STATUS_PERIODIC, bytes, duration, bitrate, duration, pidMap, tsErrors, proto, tsPerIp));
            if(bitrate / 1000000 >!100){
                emit bitrateStatus((double) bitrate /1000000,  duration );

            }
         //   emit bitrateStatus((double) bitrate /1000000,  duration );
            qInfo() << "bitrate" << (double) bitrate /1000000;
            emit workerStatus(WorkerStatus(WorkerStatus::STATUS_PERIODIC, streams), false);
            statusTimer.restart();
        }

        if (input.type == PcapProduct::END || input.type == PcapProduct::STOP) {
            break;
        }
    }
    qInfo("Analyzer done, processed %lli packets", packetNumber-1);
    emit status(AnalyzerStatus(Status::STATUS_FINISHED, bytes, duration, bitrate, duration, pidMap, tsErrors, proto, tsPerIp));

    emit workerStatus(WorkerStatus(WorkerStatus::STATUS_FINISHED, streams), false);
    if (input.type == PcapProduct::END &&
            (config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_LIVE ||
             config.getWorkerMode() == WorkerConfiguration::ANALYSIS_MODE_OFFLINE)) {

        emit workerStatus(WorkerStatus(WorkerStatus::STATUS_ANALYZED_ENTIRE, streams), false);
    }
}

PcapProduct AnalyzerPcapMiddleware::getProduct() {
    return buffer.pop();
}
