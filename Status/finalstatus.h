#ifndef FINALSTATUS_H
#define FINALSTATUS_H

#include "analyzerstatus.h"
#include "Configuration/workerconfiguration.h"
#include <QObject>
#include <QDateTime>

class FinalStatus : public AnalyzerStatus {
    Q_OBJECT

private:
    WorkerConfiguration::WorkerMode statusMode = WorkerConfiguration::NORMAL_MODE;

public:
    FinalStatus() : AnalyzerStatus() {}
    FinalStatus(Status::StatusType type) : AnalyzerStatus(type) {}
    FinalStatus(StatusType type, qint64 bytes, qint64 elapsed) :
        AnalyzerStatus(type, bytes, elapsed) {}
    FinalStatus(StatusType type, qint64 bytes, qint64 elapsed, qint64 bitrate) :
        AnalyzerStatus(type, bytes, elapsed, bitrate) {}
    ~FinalStatus() {}
    FinalStatus(FinalStatus const &other) :
        AnalyzerStatus(other), statusMode(other.statusMode) {}

    void setAnalysisMode(WorkerConfiguration::WorkerMode mode) {
        this->statusMode = mode;
    }

    void setBase(Status &status) {
        type = status.getType();
        bytes = status.getBytes();
        elapsed = status.getElapsed();
        bitrate = status.getBitrate();
    }

    void setAnalyzerInfo(AnalyzerStatus &aStatus) {
        setPidInfo(aStatus);
        this->duration = aStatus.getDuration();
        this->tsPerIp = aStatus.getTsPerIp();
        this->proto = aStatus.getProtocol();
    }

    void setPidInfo(AnalyzerStatus &aStatus) {
        this->pidMap = aStatus.getPidMap();
        this->tsErrors = aStatus.getTsErrors();
    }

    void setType(StatusType type) {
        this->type = type;
    }

    void setError(QString error) {
        this->error = error;
    }

    void setBytes(qint64 bytes) {
        this->bytes = bytes;
    }

    void setElapsed(qint64 elapsed) {
        this->elapsed = elapsed;
    }

    void setBitrate(qint64 bitrate) {
        this->bitrate = bitrate;
    }

    QString getCurrentBitrateMbit(int decimals = 2) {
        return QString::number(bitrate/1000000.0, 'f', decimals);
    }

    QString getAnalyzerInfo() {
        QString res;
        QString protoName;
        switch(proto) {
            case UDP:
                protoName = "UDP";
                break;
            case RTP:
                protoName = "RTP";
                break;
            case RTP_FEC:
                protoName = "RTP_FEC";
                break;
            default:
                protoName = "Unkown Protocol";
        }
        res.append(QString(tr("%1, %2 TS/IP")).arg(protoName).arg(tsPerIp));
        if (tsErrors.errorCounter.value(TsErrors::Continuity_count_error) >  0) {
            res.append(QString(tr("\n%1 CC Errors"))
                       .arg(tsErrors.errorCounter.value(TsErrors::Continuity_count_error)));
        }
        return res;
    }

    QString toUiString() {
        if (error.length() > 0)
            return error;
        if (statusMode == WorkerConfiguration::ANALYSIS_MODE_OFFLINE) {
            switch(type) {
                case STATUS_PERIODIC:
                    if (elapsed == 0 || duration <= 0 || bytes <= 0) return QString(tr("No data to read"));
                    return QString(tr("Analyzing... processed %1 \n%2MB - average %3 Mbit/s \n%4"))
                            .arg( QDateTime::fromTime_t(duration/1000).toUTC().toString("HH:mm:ss") )
                            .arg( QString::number((bytes/1000000.0), 'f', 2) )
                            .arg( QString::number((bytes*8/1000000.0)/(duration/1000.0), 'f', 2) )
                            .arg( getAnalyzerInfo() );
                case STATUS_FINISHED:
                    if (elapsed == 0 || duration <= 0 || bytes <= 0) return QString(tr("No data to read"));
                    return QString(tr("Analyzed: %1 \n%2MB - average %3 Mbit/s \n%4"))
                            .arg( QDateTime::fromTime_t(duration/1000).toUTC().toString("HH:mm:ss") )
                            .arg( QString::number((bytes/1000000.0), 'f', 2) )
                            .arg( QString::number((bytes*8/1000000.0)/(duration/1000.0), 'f', 2) )
                            .arg( getAnalyzerInfo() );
                default:
                    return QString(tr("No status is setup for analyzation of this type: %1")).arg(type);
            }
        }
        if (statusMode == WorkerConfiguration::ANALYSIS_MODE_LIVE) {
            switch(type) {
                case STATUS_PERIODIC:
                    if (elapsed == 0 || duration <= 0 || bytes <= 0) return QString(tr("No bitrate detected"));
                    return QString(tr("Analyzing... \n %1 Mbit/s \n%2"))
                            .arg( QString::number(bitrate/1000000.0, 'f', 2) )
                            .arg( getAnalyzerInfo() );
                case STATUS_FINISHED:
                    if (elapsed == 0 || duration <= 0 || bytes <= 0) return QString(tr("No bitrate detected"));
                        return QString(tr("Analyzing... \n average %1 Mbit/s \n%2"))
                            .arg( QString::number((bytes*8/1000000.0)/(duration/1000.0), 'f', 2) )
                            .arg( getAnalyzerInfo() );
                default:
                    return QString(tr("No status is setup for analyzation of this type: %1")).arg(type);
            }
        }
        switch(type) {
            case STATUS_STARTED:
                return QString(tr("Starting"));
            case STATUS_PERIODIC:
                return QString(tr("%1 \n%2MB - %3 Mbit/s \n%4"))
                        .arg( QDateTime::fromTime_t(elapsed/1000).toUTC().toString("HH:mm:ss") )
                        .arg( QString::number((bytes/1000000.0), 'f', 2) )
                        .arg( QString::number(bitrate/1000000.0, 'f', 2) )
                        .arg( getAnalyzerInfo() );
            case STATUS_FINISHED:
                if (elapsed == 0 || duration <= 0 || bytes <= 0) return QString(tr("No data to read"));
                return QString(tr("Finished %1 \n%2MB - average %3 Mbit/s \n%4"))
                        .arg( QDateTime::fromTime_t(elapsed/1000).toUTC().toString("HH:mm:ss") )
                        .arg( QString::number((bytes/1000000.0), 'f', 2) )
                        .arg( QString::number((bytes*8/1000000.0)/(elapsed/1000.0), 'f', 2) )
                        .arg( getAnalyzerInfo() );
            default:
                return QString(tr("No status is setup for this type: %1")).arg(type);
        }
    }
};

Q_DECLARE_METATYPE(FinalStatus)

#endif // FINALSTATUS_H
