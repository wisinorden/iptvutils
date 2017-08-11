#ifndef ANALYZERSTATUS_H
#define ANALYZERSTATUS_H

#include "status.h"
#include "Middleware/pidinfo.h"
#include "Analyzer/tserrors.h"
#include <QMap>

class AnalyzerStatus : public Status
{
public:
    enum Protocol{
        UNKNOWN,
        UDP,
        RTP,
        RTP_FEC
    };
protected:
    qint64 duration;

    QMap<int, PIDInfo> pidMap;
    TsErrors tsErrors;

    Protocol proto = UNKNOWN;
    quint8 tsPerIp = 0;

public:
    AnalyzerStatus() : Status() {}
    AnalyzerStatus(Status::StatusType type) : Status(type) {}
    AnalyzerStatus(StatusType type, qint64 bytes, qint64 elapsed ) :
        Status(type, bytes, elapsed) {}
    AnalyzerStatus(StatusType type, qint64 bytes, qint64 elapsed, qint64 bitrate) :
        Status(type, bytes, elapsed, bitrate) {}
    AnalyzerStatus(StatusType type,
                   qint64 duration,
                   QMap<int, PIDInfo> pidMap,
                   TsErrors tsErrors,
                   Protocol proto = UNKNOWN,
                   quint8 tsPerIp = -1) :
        Status(type),
        duration(duration),
        pidMap(pidMap),
        tsErrors(tsErrors),
        proto(proto),
        tsPerIp(tsPerIp)
    {}
    AnalyzerStatus(StatusType type,
                   qint64 bytes,
                   qint64 elapsed,
                   qint64 bitrate,
                   qint64 duration,
                   QMap<int, PIDInfo> pidMap,
                   TsErrors tsErrors,
                   Protocol proto = UNKNOWN,
                   quint8 tsPerIp = -1) :
        Status(type, bytes, elapsed, bitrate),
        duration(duration),
        pidMap(pidMap),
        tsErrors(tsErrors),
        proto(proto),
        tsPerIp(tsPerIp)
    {}
    ~AnalyzerStatus() {}
    AnalyzerStatus(AnalyzerStatus const &other) :
        Status(other),
        duration(other.duration),
        pidMap(other.pidMap),
        tsErrors(other.tsErrors),
        proto(other.proto),
        tsPerIp(other.tsPerIp) {}

    void setTsPerIp(quint8 tsPerIp) { this->tsPerIp = tsPerIp; }
    void setProtocol(Protocol proto) { this->proto = proto; }

    qint64 getDuration() const { return duration; }

    QMap<int, PIDInfo> getPidMap() {
        return pidMap;
    }
    TsErrors getTsErrors() {
        return tsErrors;
    }

    quint8 getTsPerIp() const { return this->tsPerIp; }
    Protocol getProtocol() const { return this->proto; }

};

Q_DECLARE_METATYPE(AnalyzerStatus)

#endif // ANALYZERSTATUS_H
