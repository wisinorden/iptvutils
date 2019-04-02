#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include <QDateTime>

class Status : public QObject {
    Q_OBJECT
public:
    enum StatusType {
        STATUS_STARTED,
        STATUS_PERIODIC,
        STATUS_FINISHED,
        STATUS_ERROR
    };

protected:
    StatusType type = STATUS_ERROR;
    qint64 bytes = 0;
    qint64 elapsed = 0;
    qint64 bitrate = 0;
    qint8 networkJitters = 0;
    QString error = "";

public:
    Status() : QObject() {}
    Status(StatusType type) : QObject(), type(type) {}
    Status(QString error) : QObject(), type(STATUS_ERROR), error(error) {}
    Status(StatusType type, qint64 bytes, qint64 elapsed) :
        QObject(), type(type), bytes(bytes), elapsed(elapsed) {}
    Status(StatusType type, qint64 bytes, qint64 elapsed, qint64 bitrate) :
        QObject(),
        type(type),
        bytes(bytes),
        elapsed(elapsed),
        bitrate(bitrate) {}
    Status(StatusType type, qint8 networkJitters) :
    type(type), networkJitters(networkJitters){}
    ~Status() {}
    Status(Status const &other) :
        QObject(),
        type(other.type),
        bytes(other.bytes),
        elapsed(other.elapsed),
        bitrate(other.bitrate),
        error(other.error) {}

    StatusType getType() { return type; }
    qint64 getBytes() { return bytes; }
    qint64 getElapsed() { return elapsed; }
    qint64 getBitrate() { return bitrate; }
    QString getError() { return error; }
};

Q_DECLARE_METATYPE(Status)

#endif // STATUS_H
