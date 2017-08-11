#ifndef STREAMID_H
#define STREAMID_H

#include <QString>
#include <QStringList>
#include "packetparser.h"

class StreamId {
public:
    // Unique id for the stream, IPv4 address + port (48 lowest bits)
    quint64 id;
    QString name;

public:
    StreamId() {}
    StreamId(QString host, quint16 port) : id(0), name("") {
        id = calcId(host, port);

        name = QString("%1:%2").arg(host).arg(port);

        // Validation that name and id match
        Q_ASSERT(name == calcName());
    }
    StreamId(quint64 id) : id(id), name("") {
        name = calcName();
    }

    QString getHost() const { return name.split(":").at(0); }
    quint16 getPort() const { return name.split(":").at(1).toUShort(); }

    quint64 getId() const { return id; }
    static quint64 calcId(QString host, quint16 port) {
        quint64 id = 0;
        QStringList ipParts = host.split(".");
        for (int i = 0; i < 4; i++) {
            quint64 part = (quint8)(ipParts.at(i).toShort()&0xFF) << (8*(3-i));
            //qInfo("part %i, 0x%x", i, part);
            id |= part;
        }
        //qInfo("port %x", port);
        id = id << 16;
        id += port;
        id &= 0xFFFFFFFFFFFF;
        return id;
    }
    static quint64 calcId(QString hostport) {
        QStringList parts = hostport.split(":");
        return calcId(parts.at(0), parts.at(1).toUShort());
    }
    static quint64 calcId(ip_address ip, quint64 port) {
        quint64 id = 0;
        id |= ((quint64)ip.byte1 << 40);
        id |= ((quint64)ip.byte2 << 32);
        id |= ((quint64)ip.byte3 << 24);
        id |= ((quint64)ip.byte4 << 16);
        id += port;
        return id;
    }
    QString getName() const { return name; }
    QString calcName() const {
        return QString("%1.%2.%3.%4:%5")
                .arg( (id & 0xFF0000000000) >> 40 )
                .arg( (id & 0x00FF00000000) >> 32 )
                .arg( (id & 0x0000FF000000) >> 24 )
                .arg( (id & 0x000000FF0000) >> 16 )
                .arg( (id & 0x00000000FFFF) );
    }
    static QString calcName(quint64 id) {
        return QString("%1.%2.%3.%4:%5")
                .arg( (id & 0xFF0000000000) >> 40 )
                .arg( (id & 0x00FF00000000) >> 32 )
                .arg( (id & 0x0000FF000000) >> 24 )
                .arg( (id & 0x000000FF0000) >> 16 )
                .arg( (id & 0x00000000FFFF) );
    }


};

inline bool operator==(const StreamId& lhs, const StreamId& rhs) {
    return lhs.getId() == rhs.getId();
}

inline bool operator<(const StreamId& lhs, const StreamId& rhs) {
    return lhs.getId() < rhs.getId();
}

inline uint qHash(const StreamId &key) {
    return qHash(key.getId());
}

#endif // STREAMID_H
