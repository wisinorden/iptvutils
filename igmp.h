#ifndef IGMP_H
#define IGMP_H

#include <QString>
#include <QUdpSocket>

class IGMP
{
private:
    static QUdpSocket *udpSocket;
public:
    IGMP() = delete;
    static void init();
    static bool joinMulticastGroup(QString host);
    static bool leaveMulticastGroup(QString host);
};

#endif // IGMP_H
