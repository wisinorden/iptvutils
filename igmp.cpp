#include "igmp.h"

QUdpSocket *IGMP::udpSocket = new QUdpSocket();

void IGMP::init()
{
    udpSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress);
}

bool IGMP::joinMulticastGroup(QString host)
{
    QHostAddress addr(host);
    qInfo("joining %s", qPrintable(host));
    return udpSocket->joinMulticastGroup(addr);
}

bool IGMP::leaveMulticastGroup(QString host)
{
    QHostAddress addr(host);
    qInfo("leaving %s", qPrintable(host));
    return udpSocket->leaveMulticastGroup(addr);
}
