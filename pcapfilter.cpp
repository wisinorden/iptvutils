#include "pcapfilter.h"

#include <QStringList>

PcapFilter::PcapFilter()
{

}

QString PcapFilter::generateFilter(QString host, unsigned short port, bool rtpFec) {
    QString ports;
    if (rtpFec) {
        ports = QString("(port %1 or port %2 or port %3)").arg(port).arg(port+2).arg(port+4);
    }
    else {
        ports = QString("port %1").arg(port);
    }
    return QString("host %1 and %2 and proto UDP").arg(host).arg(ports);
}

QString PcapFilter::generateFilter(QString hostport, bool rtpFec) {
    QStringList parts = hostport.split(":");
    return PcapFilter::generateFilter(parts[0], parts[1].toUShort(), rtpFec);
}
