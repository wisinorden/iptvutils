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
    static bool joinMulticastGroup(const QString &host, const QNetworkInterface &iface);
    static bool leaveMulticastGroup(const QString &host, const QNetworkInterface &iface);
};

#endif // IGMP_H
