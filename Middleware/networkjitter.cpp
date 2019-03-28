#include "networkjitter.h"
#include <QElapsedTimer>

#include <iostream>
#include "Status/streamid.h"
#include "packetparser.h"
#include "tsparser.h"
#include <QDebug>
#include <QtMath>

//This class is measuring eventual network jitter

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
    qint64 differencePerSec = 0;
    qint64 packetCounter = 0;
    qint16 diffratePerSec = 0;

    qint64 diffSum = 0;
    qint64 finalSum = 0;


    QList<qint16> distanceList;



    qint64 bytes = 0;
    //    AnalyzerStatus::Protocol proto = AnalyzerStatus::UNKNOWN;

//    bool hasLooped = false;

    PcapProduct input;
    PcapProduct previousInput = prevProvider->getProduct();
    buffer.push(previousInput);




    while (!stopping) {
        PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
        bytes += parser.data_len;


        quint64 streamId = StreamId::calcId(parser.ih->daddr, parser.dport);

        input = prevProvider->getProduct();



        //if (input.type == PcapProduct::NORMAL && !hasLooped)
        {
            PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
            bytes += parser.data_len;


            inputTs = ((pcap_pkthdr*)input.header.data())->ts.tv_sec * 1000000;
            inputTs += ((pcap_pkthdr*)input.header.data())->ts.tv_usec;

            previousInputTs = ((pcap_pkthdr*)previousInput.header.data())->ts.tv_sec * 1000000;
            previousInputTs += ((pcap_pkthdr*)previousInput.header.data())->ts.tv_usec;

            difference = inputTs - previousInputTs;
            differencePerSec += difference;
            distanceList.append(difference);
            //qInfo() << difference;



            if (difference != 0){
                //   qInfo() << difference;
            } else {

            }


            //Calculate average diff per second

            if(statusTimer.elapsed() >= 1000){
                diffratePerSec = differencePerSec/packetCounter;


                for( int a = 0; a < distanceList.length(); a = a + 1 ) {

                    diffSum += ((distanceList[a] - diffratePerSec) * (distanceList[a] - diffratePerSec));
                }

                finalSum = diffSum/ distanceList.length();

                qInfo() << "Std deviation: " << sqrt(finalSum);

                packetCounter = 0;
                diffSum = 0;
                finalSum = 0;
                distanceList.clear();

                differencePerSec = 0;
                statusTimer.restart();

            }






        }
        previousInput = input;
        buffer.push(input);
        packetNumber++;
        packetCounter ++;


        if (input.type == PcapProduct::END || input.type == PcapProduct::STOP) {
            break;
        }
#include "Status/streamid.h"

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
