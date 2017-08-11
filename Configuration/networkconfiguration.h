#ifndef NETWORKCONFIGURATION_H
#define NETWORKCONFIGURATION_H

#include <QString>
#include "interface.h"

class NetworkConfiguration
{
protected:
    Interface device;
    const QString host;
    const quint16 port;
    NetworkConfiguration() : host(""), port(0) {}
    NetworkConfiguration(Interface device, QString host, quint16 port) : device(device), host(host), port(port) {}

public:
    Interface getDevice() const { return device; }
    QString getHost() const { return host; }
    quint16 getPort() const { return port; }
};

#endif // NETWORKCONFIGURATION_H
