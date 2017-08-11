#ifndef RECORDWIDGET_H
#define RECORDWIDGET_H

#include "Status/finalstatus.h"
#include "Recorder/networkpcapfilerecorder.h"
#include "Recorder/tsnetworkfilerecorder.h"
#include <QWidget>

namespace Ui {
class RecordWidget;
}

class RecordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordWidget(QWidget *parent = 0);
    ~RecordWidget();

    void loadSettings();
    void saveSettings();

    static NetworkPcapFileRecorder *networkPcapFileRecorder;
    static TsNetworkFileRecorder *tsNetworkFileRecorder;

private:
    Ui::RecordWidget *ui;

private slots:
    void recordingStarted();
    void recordingFinished();
    bool validateRecordInputs();
    void recordFilterShouldUpdate();

    bool startPcapRecord(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);
    bool startTsRecord(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);
    void on_recordOpenFileDialog_clicked();
    void on_startPcapRecordBtn_clicked();
    void on_startTsRecordBtn_clicked();
    void on_recordCancelBtn_clicked();

public slots:
    void recordStatusChanged(FinalStatus status);
    void recordWorkerStatusChanged(WorkerStatus status);

};

#endif // RECORDWIDGET_H
