#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QThread>
#include "../Configuration/workerconfiguration.h"
#include "Status/finalstatus.h"
#include "Status/workerstatus.h"

class Player : public QObject
{
    Q_OBJECT
protected:
    WorkerConfiguration config;
    QString playerName = QString("Player");
    QThread producerThread;
    QThread consumerThread;
    FinalStatus finalStatus;

public:
    Player(WorkerConfiguration config, QObject *parent = 0) : QObject(parent), config(config) {}
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void stopAndWait() = 0;
    // each type of player implements a way to start playback
    //virtual void startPlayback(QObject* requester) = 0;

signals:
    void started();
    void finished();
    void status(FinalStatus status);
    void workerStatus(WorkerStatus status);

public slots:
    virtual void gotProducerStatus(Status status) {Q_UNUSED(status);}
    virtual void gotConsumerStatus(Status status) {Q_UNUSED(status);}
};

#endif // PLAYER_H
