#ifndef WORKERSTATUS_H
#define WORKERSTATUS_H

#include "streamid.h"
#include "streaminfo.h"

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QTreeWidget>

class WorkerStatus : public QObject {
    Q_OBJECT
public:
    enum StatusType {
        STATUS_STARTED,
        STATUS_PERIODIC,
        STATUS_FINISHED,
        STATUS_ERROR,
        STATUS_ANALYZED_ENTIRE
    };

protected:
    StatusType type = STATUS_ERROR;
    qint64 networkJitters;
    QString error = "";
    QHash<quint64, StreamInfo> streams;

    QTreeWidgetItem* makeItem(QString text, bool parent = false) {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(text));
        item->setDisabled(!parent);
        return item;
    }

    void makeTsError(QTreeWidgetItem *parent, quint64 count, TsErrors::ErrorType error) {
        QTreeWidgetItem *item = new QTreeWidgetItem(
                    (QTreeWidget*)0,
                    QStringList(QString("%1: %2")
                                .arg(TsErrors::errorName(error))
                                .arg(count)));
        parent->addChild(item);
        item->setDisabled(true);
    }




public:
    WorkerStatus() : QObject() {}
    WorkerStatus(StatusType type) : QObject(), type(type) {}
    WorkerStatus(StatusType type, qint64 networkJitters) :
    type(type), networkJitters(networkJitters){}
    WorkerStatus(QString error) : QObject(), type(STATUS_ERROR), error(error) {}
    WorkerStatus(StatusType type, QHash<quint64, StreamInfo> streams) :
        QObject(),
        type(type),
        streams(streams) {}
    ~WorkerStatus() {}
    WorkerStatus(WorkerStatus const &other) :
        QObject(),
        type(other.type),
        streams(other.streams) {}

    StatusType getType() { return type; }
    QString getError() { return error; }
    QHash<quint64, StreamInfo> getStreams() { return streams; }

    void insertIntoTree(QTreeWidget *tree) {
        tree->clear();
        tree->setSelectionMode(QAbstractItemView::SingleSelection);
        for (int i = 0; i < streams.count(); i++) {
            quint64 key = streams.keys().at(i);
            const StreamInfo &info = streams.value(key);

            QTreeWidgetItem* parent = makeItem(StreamId::calcName(key), true);
            tree->addTopLevelItem(parent);
            parent->addChild(makeItem(QString(tr("size %1 MB")).arg(QString::number(info.bytes/1000000.0, 'f', 2))));
            parent->addChild(makeItem(QString(tr("duration %1")).arg(QDateTime::fromTime_t(info.currentTime/1000).toUTC().toString("HH:mm:ss"))));
            parent->addChild(makeItem(QString(tr("avg bitrate %1 Mbit/s")).arg(QString::number((info.bytes*8*1000.0/info.currentTime)/1000000.0, 'f', 2))));
            parent->addChild(makeItem(QString(tr("protocol %1")).arg(info.protocolName())));
            parent->addChild(makeItem(QString(tr("bitrate mode %1")).arg(info.bitrateModeName())));
            parent->addChild(makeItem(QString(tr("%1 TS/IP")).arg(info.tsPerIp)));
            parent->addChild(makeItem(QString(tr("%1 std IAT deviation")).arg(info.networkJitters)));
            parent->addChild(makeItem(QString(tr("%1 PIDs")).arg(info.pidMap.size())));

            quint64 tsErrCount = info.tsErrors.totalErrors();
            QTreeWidgetItem* tserrors = makeItem(QString(tr("TsErrors (%1)")).arg(tsErrCount));
            parent->addChild(tserrors);
            if (tsErrCount > 0) tserrors->setExpanded(true);
            QMapIterator<TsErrors::ErrorType, qint64> it(info.tsErrors.errorCounter);
            while(it.hasNext()) {
                it.next();
                makeTsError(tserrors, it.value(), it.key());
            }

            parent->setExpanded(true);

            tree->clearSelection();
            parent->setSelected(true);
        }

    }
};

Q_DECLARE_METATYPE(WorkerStatus)


#endif // WORKERSTATUS_H
