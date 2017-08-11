#ifndef NETWORKINPUTCONFIGURATION_H
#define NETWORKINPUTCONFIGURATION_H

#include "networkconfiguration.h"

class NetworkInputConfiguration : public NetworkConfiguration
{
private:
    const QString filter;
public:
    NetworkInputConfiguration() : filter("") {}
    NetworkInputConfiguration(Interface device, QString host, quint16 port, QString filter) : NetworkConfiguration(device, host, port), filter(filter) {}

    QString getFilter() const { return filter; }
};

#endif // NETWORKINPUTCONFIGURATION_H
