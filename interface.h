#ifndef INTERFACE_H
#define INTERFACE_H

#include <QString>
#include <QNetworkInterface>
#include <QHostAddress>

extern "C" {
    #include <pcap.h>
}

class Interface
{
private:
    QString id;
    QString name;
    QNetworkInterface qInterface;
    QHostAddress address;
    quint64 mac;
public:
    Interface() = default;
    Interface(QString id, QString name, QNetworkInterface qInterface, QHostAddress address, quint64 mac);

    static bool getInterfaces(QList<Interface>* interfaces);

    QString getId() const;
    QString getName() const;
    QNetworkInterface getQInterface() const;
    QHostAddress getAddress() const;
    quint64 getMac() const;
};

#endif // INTERFACE_H
