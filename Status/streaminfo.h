#ifndef STREAMINFO_H
#define STREAMINFO_H

#include "Analyzer/tserrors.h"
#include "Status/streamid.h"
#include "Middleware/pidinfo.h"
#include <QtNumeric>

class StreamInfo {
public:
    enum class NetworkProtocol{
        UNKNOWN,
        UDP,
        RTP,
        RTP_FEC
    };

    enum class BitrateMode{
        UNKNOWN,
        CBR,
        VBR
    };

public:
    StreamId id;
    quint64 bytes;
    quint64 currentTime;
    quint64 bitrate;
    double avgBitrate;
    double currentBitrate;
    NetworkProtocol protocol;
    BitrateMode bitrateMode;
    quint8 tsPerIp;
    quint64 iatDeviation;
    quint64 lastSecondBytes;
    quint64 lastDuration;
    TsErrors tsErrors;
    QMap<int, PIDInfo> pidMap;
    QString protocolName() const {
        switch (this->protocol) {
            case NetworkProtocol::UDP:
                return QString("UDP");
            case NetworkProtocol::RTP:
                return QString("RTP");
            case NetworkProtocol::RTP_FEC:
                return QString("RTP_FEC");
            default:
                return QString("?");
        }
    }
    QString bitrateModeName() const {
        switch (this->bitrateMode) {
            case BitrateMode::CBR:
                return QString("CBR");
            case BitrateMode::VBR:
                return QString("VBR");
            default:
                return QString("?");
        }
    }

    StreamInfo() :
        id(),
        bytes(0),
        currentTime(0),
        bitrate(0),
        protocol(NetworkProtocol::UNKNOWN),
        bitrateMode(BitrateMode::UNKNOWN),
        tsPerIp(0),
        iatDeviation(0),
        lastSecondBytes(0),
        lastDuration(0),
        tsErrors() {}
    StreamInfo(StreamId id,
               quint64 bytes,
               quint64 bitrate,
               NetworkProtocol protocol,
               quint8 tsPerIp,
               quint8 networkJitters,
               TsErrors tsErrors) :
        id(id),
        bytes(bytes),
        bitrate(bitrate),
        protocol(protocol),
        tsPerIp(tsPerIp),
        iatDeviation(networkJitters),
        tsErrors(tsErrors) {}
};

#endif // STREAMINFO_H
