#include "interface.h"
#ifdef __linux__
#include <arpa/inet.h>
#endif

Interface::Interface(QString id, QString name, QNetworkInterface qInterface, QHostAddress address, quint64 mac) {
    this->id = id;
    if (name != "") {
        this->name = QString("%1 (%2) - %3")
                .arg(name)
                .arg(qInterface.humanReadableName())
                .arg(address.toString());

    }
    else {
        this->name = QString("%1 - %3")
                .arg(qInterface.humanReadableName())
                .arg(address.toString());
    }
    this->qInterface = qInterface;
    this->address = address;
    this->mac = mac;
}

#define IPTOSBUFFERS    12
char *iptos(u_long in)
{
    static char output[IPTOSBUFFERS][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

bool Interface::getInterfaces(QList<Interface>* interfaces)
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* Retrieve the pcap device list */
    if(pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        qFatal("Error in pcap_findalldevs: %s\n", errbuf);
        return false;
    }

    /* Retrieve the Qt device list */
    QList<QNetworkInterface> qInterfaces = QNetworkInterface::allInterfaces();

    pcap_addr_t *a;

    /* Write to list */
    for(d=alldevs; d; d=d->next)
    {
        for (a = d->addresses; a; a=a->next) {
            quint32 pcapIp = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;

            for (int i = 0; i < qInterfaces.count(); i++) {
                QList<QNetworkAddressEntry> entries = qInterfaces.at(i).addressEntries();
                for (int j = 0; j < entries.count(); j++) {
                    if (pcapIp != 0 && entries.at(j).ip().toIPv4Address() != 0 &&
                            ntohl(pcapIp) == entries.at(j).ip().toIPv4Address()) {

                        QString macString = qInterfaces.at(i).hardwareAddress();
                        if (macString.length() != 17) {
                            qFatal("MAC ADDRESS FAILED TO RESOLVE");
                            return false;
                        }

                        QStringList splittedMac = macString.split(':');
                        quint64 mac = 0;
                        for (int i = 0; i < 6; i++) {
                            bool ok;
                            mac |= (splittedMac.at(5-i).toULongLong(&ok, 16) << 8*i);
                        }

                        qInfo("Interface: %s(%s) for address: %s, Mac: %s (0x%llX)",
                              qPrintable(qInterfaces.at(i).humanReadableName()),
                              d->description,
                              iptos(pcapIp),
                              qPrintable(macString),
                              mac);

                        interfaces->append(Interface(
                                               QString::fromLocal8Bit(d->name),
                                               QString::fromLocal8Bit(d->description),
                                               qInterfaces.at(i),
                                               entries.at(j).ip(),
                                               mac));
                    }

                }
            }

        }
    }
    return true;
}

QString Interface::getId() const
{
    return this->id;
}

QString Interface::getName() const
{
    return this->name;
}

QNetworkInterface Interface::getQInterface() const {
    return this->qInterface;
}

QHostAddress Interface::getAddress() const {
    return this->address;
}

quint64 Interface::getMac() const {
    return this->mac;
}
