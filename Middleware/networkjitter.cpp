#include "networkjitter.h"
#include <QElapsedTimer>

#include <iostream>
#include <QDebug>


void NetworkJitter::init() {
    this->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::started, this, &NetworkJitter::run);
    connect(this, &NetworkJitter::finished, &runnerThread, &QThread::quit);
}



void NetworkJitter::run(){
    emit started();
    QElapsedTimer statusTimer;
    statusTimer.start();
    packetNumber = 0;


    qint64 inputTs = 0;
    qint64 previousInputTs = 0;
    qint64 difference = 0;



    qint64 bytes = 0;
    //    AnalyzerStatus::Protocol proto = AnalyzerStatus::UNKNOWN;

    bool hasLooped = false;

    PcapProduct input;
    PcapProduct previousInput = prevProvider->getProduct();
    buffer.push(previousInput);

    while (!stopping) {
        input = prevProvider->getProduct();
        //if (input.type == PcapProduct::NORMAL && !hasLooped)
        {
            PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
            bytes += parser.data_len;


            inputTs = ((pcap_pkthdr*)input.header.data())->ts.tv_sec * 1000;
            inputTs += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000;

            previousInputTs = ((pcap_pkthdr*)previousInput.header.data())->ts.tv_sec * 1000;
            previousInputTs += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000;

            difference = inputTs - previousInputTs;
            qInfo() << difference;

        }
        previousInput = input;
        buffer.push(input);
        packetNumber++;

        if (input.type == PcapProduct::END || input.type == PcapProduct::STOP) {
            break;
        }

    }

emit finished();

}



void NetworkJitter::start() {
    runnerThread.start();
//    run();

}


PcapProduct NetworkJitter::getProduct() {
    return buffer.pop();
}


void NetworkJitter::stop() {
    stopping = true;
    buffer.stop();
}
