#ifndef TSERRORS_H
#define TSERRORS_H

#include <QMap>
#include <QMetaEnum>
#include <QMetaObject>

class TsErrors {

public:
    // TS 101 290 monitor indicators
    enum ErrorType {
        // First priority
        TS_sync_loss,
        Sync_byte_error,
        PAT_error_2,
        Continuity_count_error,
        PMT_error_2,
        PID_error,

        // Second priority
        Transport_error,
        CRC_error,
        PCR_repetition_error,
        PCR_discontinuity_indicator_error,
        PCR_accuracy_error,
        PTS_error,
        CAT_error
    };

    TsErrors() :
        errorCounter() {}
    ~TsErrors() {}
    TsErrors(const TsErrors &other) :
        errorCounter(other.errorCounter) {}

    QMap<ErrorType, qint64> errorCounter;

    quint64 totalErrors() const {
        quint64 errors = 0;
        QMapIterator<ErrorType, qint64> i(errorCounter);
        while (i.hasNext()) {
            i.next();
            errors += i.value();
        }
        return errors;
    }

    static QString errorName(ErrorType type) {
        switch(type) {
            // First priority
            case TS_sync_loss:      return QString("TS_sync_loss");
            case Sync_byte_error:   return QString("Sync_byte_error");
            case PAT_error_2:       return QString("PAT_error_2");
            case Continuity_count_error: return QString("Continuity_count_error");
            case PMT_error_2:       return QString("PMT_error_2");
            case PID_error:         return QString("PID_error");

            // Second priority
            case Transport_error:   return QString("Transport_error");
            case CRC_error:         return QString("CRC_error");
            case PCR_repetition_error: return QString("PCR_repetition_error");
            case PCR_discontinuity_indicator_error: return QString("PCR_discontinuity_indicator_error");
            case PCR_accuracy_error: return QString("PCR_accuracy_error");
            case PTS_error:         return QString("PTS_error");
            case CAT_error:         return QString("CAT_error");

            default:
                return QString("Unknown");
        }
    }
};

#endif // TSERRORS_H
