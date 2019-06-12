#ifndef RECORDER_H
#define RECORDER_H

#include <QThread>
#include "../Configuration/workerconfiguration.h"
#include "Status/finalstatus.h"
#include "Status/workerstatus.h"

class Recorder : public QObject
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    QThread producerThread;
    QThread consumerThread;
    FinalStatus finalStatus;
    bool stopping = false;

public:
    Recorder(WorkerConfiguration config, QObject *parent = 0) : QObject(parent), config(config) {}
    virtual void start() = 0;
    virtual void stop() { stopping = true; }

signals:
    void started();
    void finished();
    void status(FinalStatus status);
    void bitrateStatus(double bitrate, qint64 duration);
    void iatStatus(double iatDev, qint64 duration);
    void workerStatus(WorkerStatus status);

public slots:
    virtual void gotProducerStatus(Status pStatus) {Q_UNUSED(pStatus);}
    virtual void gotAnalyzerStatus(AnalyzerStatus aStatus) {Q_UNUSED(aStatus);}
    virtual void gotConsumerStatus(Status cStatus) {Q_UNUSED(cStatus);}
    virtual void moduleFinished() {}
};

#endif // RECORDER_H
