#include "networkjitter.h"
#include <QElapsedTimer>

#include <iostream>
#include "Status/streamid.h"
#include "Status/streaminfo.h"
#include "packetparser.h"
#include "tsparser.h"
#include <QDebug>
#include <QtMath>

//This class is for measuring network jitter

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
    qint64 startTime = -1;
    qint64 endTime = -1;


    QList<qint16> distanceList;
    qint64 bytes = 0;
    PcapProduct input;





    PcapProduct previousInput = prevProvider->getProduct();

    buffer.push(previousInput);




    while (!stopping) {
        PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
        bytes += parser.data_len;
        input = prevProvider->getProduct();


        {

            PacketParser parser((pcap_pkthdr*)input.header.data(), (const u_char*)input.data.data());
            bytes += parser.data_len;

            // Takes packet Timestamp and compares it to the previous one to calculate difference.
            inputTs = ((pcap_pkthdr*)input.header.data())->ts.tv_sec * 1000000;
            inputTs += ((pcap_pkthdr*)input.header.data())->ts.tv_usec;

            previousInputTs = ((pcap_pkthdr*)previousInput.header.data())->ts.tv_sec * 1000000;
            previousInputTs += ((pcap_pkthdr*)previousInput.header.data())->ts.tv_usec;



            if (startTime == -1) {
                startTime = ((pcap_pkthdr*)input.header.data())->ts.tv_sec * 1000; // convert to ms
                startTime += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000; // convert to ms
            }
            else {
                endTime = ((pcap_pkthdr*)input.header.data())->ts.tv_sec*1000;
                endTime += ((pcap_pkthdr*)input.header.data())->ts.tv_usec/1000; // convert to ms
                duration = endTime - startTime;

            }





            difference = inputTs - previousInputTs;
            differencePerSec += difference;
            distanceList.append(difference);

            //Calculate average diff per second and IAT standard deviation per millisecond

            if(statusTimer.elapsed() >= 1000){
                // If no packages recieved, wait(?)
                if(packetCounter != 0){
                diffratePerSec = differencePerSec/packetCounter;
                }

                for( int a = 0; a < distanceList.length(); a = a + 1 ) {

                    diffSum += ((distanceList[a] - diffratePerSec) * (distanceList[a] - diffratePerSec));
                }

                // sqrt of finalSum is equal to the IAT dev

                finalSum = diffSum/ distanceList.length();

                qInfo() << "Std deviation: " << sqrt(finalSum);


                quint64 streamId = StreamId::calcId(parser.ih->daddr, parser.dport);
                StreamInfo &stream = streams[streamId];


                stream.networkJitters = sqrt(finalSum);

                emit iatStatus(sqrt(finalSum), duration);

                emit workerStatus(WorkerStatus(WorkerStatus::STATUS_PERIODIC, streams));



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

    }
    emit finished();
}



void NetworkJitter::start() {
    runnerThread.start();
}


PcapProduct NetworkJitter::getProduct() {
    return buffer.pop();
}


void NetworkJitter::stop() {
    stopping = true;
    buffer.stop();
}
