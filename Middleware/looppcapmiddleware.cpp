#include "looppcapmiddleware.h"
#include "packetparser.h"
#include "tsparser.h"

void LoopPcapMiddleware::init() {
    this->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::started, this, &LoopPcapMiddleware::run);
    connect(this, &LoopPcapMiddleware::finished, &runnerThread, &QThread::quit);
}

void LoopPcapMiddleware::start() {
    runnerThread.start();
}

void LoopPcapMiddleware::stop() {
    stopping = true;
    buffer.stop();
}

void LoopPcapMiddleware::run() {
    emit started();
    qInfo("LoopPcapMiddleware started");
    bufferProducts();
    qInfo("LoopPcapMiddleware finished");
    emit finished();
    this->thread()->quit();
}

quint64 LoopPcapMiddleware::timevalToUsec(timeval time) {
    quint64 secs = time.tv_sec;
    quint64 usecs = secs*1000000 + time.tv_usec;
    return usecs;
    //return time.tv_sec*1000000 + time.tv_usec;
}

timeval LoopPcapMiddleware::usecToTimeval(quint64 time) {
    long int seconds = time/1000000;
    long int usecs = time - seconds*1000000;
    return timeval{seconds, usecs};
}

// Buffers packets
void LoopPcapMiddleware::bufferProducts() {
    // Last value cc before loop, used to keep CC continious
    QMap<quint16, quint8> pidCCOffset;
    QMap<quint16, quint8> pidCCNewOffset;

    quint64 packetNumber = 0;

    // To keep time continious after loops, only used for consumer sleep timing
    bool firstTimeSet = false;
    quint64 startTime = 0;
    quint64 lastTime = 0; // will always be end of file when read
    quint64 timeOffset = 0;

    QSet<quint16> pcrPids;
    QMap<quint16, bool> pcrPidDiscontinuousSet;
    TsParser tsparser;

    while (!stopping) {
        PcapProduct input = prevProvider->getProduct();
        if (input.type == PcapProduct::LOOP) {
            qInfo("LOOOOP");

            if (config.getInputType() == WorkerConfiguration::FILE &&
                    config.getFileInput().getLoopStyle() == FileInputConfiguration::LOOP_DISCONTINUITY_FLAG) {
                QMapIterator<quint16, ProgramInfo> it(tsparser.programPidMap);
                while (it.hasNext()) {
                    it.next();
                    pcrPids.insert( it.value().PCR_PID );
                    pcrPidDiscontinuousSet.insert(it.value().PCR_PID, false);
                    qInfo("PCR_PID: %i", it.value().PCR_PID);
                }

                // Set offsets for this loop, constant over one loop
                QMapIterator<quint16, quint8> i(pidCCNewOffset);
                while (i.hasNext()) {
                    i.next();
                    pidCCOffset[i.key()] = i.value();
                    //qInfo("CC offset: pid 0x%x, offset %u", i.key(), i.value());
                }
                //qInfo("contain cc offsets for %i pids", pidCCOffset.size());
                timeOffset += lastTime-startTime;
                //qInfo("startTime: %lu", usecToTimeval(startTime).tv_sec);
                qInfo("timeOffset: %llu ms", timeOffset/1000);
                //qInfo("newStart: %lu", usecToTimeval(startTime+timeOffset).tv_sec);
            }
        }
        else if (input.type == PcapProduct::NORMAL) {
            packetNumber++;
            pcap_pkthdr* header = (pcap_pkthdr*)input.header.data();
            PacketParser parser(header, (const u_char*)input.data.data());

            if (config.getFileInput().getLoopStyle() == FileInputConfiguration::LOOP_DISCONTINUITY_FLAG) {
                if (!firstTimeSet) {
                    qInfo("start time sec: %lu", header->ts.tv_sec);
                    startTime = timevalToUsec(header->ts);
                    firstTimeSet = true;
                }
                lastTime = timevalToUsec(header->ts);
                header->ts = usecToTimeval(lastTime+timeOffset);
            }


            for (uint i = 0; i < parser.data_len/188; i++) {
                tsparser.parse((quint8*)(parser.data+i*188));

                if (config.getFileInput().getLoopStyle() == FileInputConfiguration::LOOP_DISCONTINUITY_FLAG) {

                    // update cc offset every time we see a new Ts-packet
                    pidCCNewOffset[tsparser.hdr.PID] = (pidCCNewOffset[tsparser.hdr.PID]+1) & 0x0F;

                    // set the offset, drops will still be kept
                    tsparser.addCCOffset(pidCCOffset[tsparser.hdr.PID]);

                    // set PCR discontinious flag if first pcr on that pid for this loop
                    if (pcrPidDiscontinuousSet.contains(tsparser.hdr.PID)) {
                        if (pcrPidDiscontinuousSet[tsparser.hdr.PID] == false) {
                            if (tsparser.hdr.AFC >= 2 && tsparser.af.PCR_flag == 1) {
                                tsparser.setDiscontiniousFlag();
                                pcrPidDiscontinuousSet[tsparser.hdr.PID] = true;
                                //qInfo("%u: discced: %i", packetNumber, tsparser.hdr.PID);
                            }
                        }
                    }
                }
            }

        }
        if (input.type == PcapProduct::LOOP &&
                config.getFileInput().getLoopStyle() == FileInputConfiguration::LOOP_DISCONTINUITY_FLAG) {
            // should not send the loop packet
            // time and cc are offseted and PCR is flagged discontinious
        }
        else {
            buffer.push(input);
        }

        if (input.type == PcapProduct::END || input.type == PcapProduct::STOP)
            break;
    }
}

PcapProduct LoopPcapMiddleware::getProduct() {
    return buffer.pop();
}
