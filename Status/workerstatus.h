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

    StatusType type = STATUS_ERROR;
    QHash<quint64, StreamInfo> streams;

protected:
    QString error = "";

    QTreeWidgetItem* makeItem(QString text, bool parent = false) {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(text));
        item->setDisabled(!parent);
        return item;
    }

    QTreeWidgetItem* makeItems(QString text, QVariant value, bool parent = false) {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(text));
        item->setData(0, Qt::UserRole, value);
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
    double value;
    QTreeWidget *treeWidget;
    WorkerStatus() : QObject() {}
    WorkerStatus(StatusType type) : QObject(), type(type) {}
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
    const QHash<quint64, StreamInfo>& getStreams() const { return streams; }




    void setStreams(const QHash<quint64, StreamInfo> &value)
    {
        streams = value;
    }


    // New method that is meant to update the values in the tree rather than recreate the tree every iteration.
    void updateTree(QTreeWidget *tree){

        quint8 y = 0;
        double value = 0;

        for (int i = 0; i < streams.count(); i++){
            quint64 key = streams.keys().at(i);
            const StreamInfo &info = streams.value(key);

            tree->topLevelItem(i)->child(y++)->setText(0, QString(tr("size %1 MB")).arg(QString::number(info.bytes/1000000.0, 'f', 2)));
            tree->topLevelItem(i)->child(y++)->setText(0, QString(tr("duration %1")).arg(QDateTime::fromTime_t(info.currentTime/1000).toUTC().toString("HH:mm:ss")));
            value = (info.bytes*8*1000.0/info.currentTime)/1000000.0;
            tree->topLevelItem(i)->child(y++)->setText(0,QString((tr("avg bitrate %1 Mbit/s")).arg(QString::number(value, 'f', 2))));
            tree->topLevelItem(i)->child(y++)->setText(0,QString(tr("protocol %1")).arg(info.protocolName()));
            tree->topLevelItem(i)->child(y++)->setText(0, (QString(tr("bitrate mode %1")).arg(info.bitrateModeName())));
            tree->topLevelItem(i)->child(y++)->setText(0, QString(tr("%1 TS/IP")).arg(info.tsPerIp));
            tree->topLevelItem(i)->child(y++)->setText(0, QString(tr("%1 µs IAT deviation ")).arg(info.iatDeviation));
            tree->topLevelItem(i)->child(y++)->setText(0, QString(tr("%1 PIDs")).arg(info.pidMap.size()));
            y= 0;
        }
    }





    // Keeping this method as the initial tree creator
    void insertIntoTree(QTreeWidget *tree) {
        tree->clear();
        tree->setSelectionMode(QAbstractItemView::SingleSelection);
        qInfo() << "ADDDING " << streams.count() << " NUMBER OF STREAMS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        for (int i = 0; i < streams.count(); i++) {
            quint64 key = streams.keys().at(i);
            const StreamInfo &info = streams.value(key);

            QTreeWidgetItem* parent = makeItem(StreamId::calcName(key), true);
            tree->addTopLevelItem(parent);

            parent->addChild(makeItem(QString(tr("size %1 MB")).arg(QString::number(info.bytes/1000000.0, 'f', 2))));
            parent->addChild(makeItem(QString(tr("duration %1")).arg(QDateTime::fromTime_t(info.currentTime/1000).toUTC().toString("HH:mm:ss"))));
            auto value = (info.bytes*8*1000.0/info.currentTime)/1000000.0;
            parent->addChild(makeItems(QString(tr("avg bitrate %1 Mbit/s")).arg(QString::number(value, 'f', 2)), value));    //Allows avg bitrate to be accessed in other places such as recordwidget
            parent->addChild(makeItem(QString(tr("protocol %1")).arg(info.protocolName())));
            parent->addChild(makeItem(QString(tr("bitrate mode %1")).arg(info.bitrateModeName())));
            parent->addChild(makeItem(QString(tr("%1 TS/IP")).arg(info.tsPerIp)));
            //       if(info.networkJitters != 0){
            parent->addChild(makeItem(QString(tr("%1 µs IAT deviation ")).arg(info.iatDeviation)));
            //          }
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
           // this->treeWidget = tree;
            tree->clearSelection();
            parent->setSelected(true);
        }

    }

};




Q_DECLARE_METATYPE(WorkerStatus)


#endif // WORKERSTATUS_H
