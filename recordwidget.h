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
    bool started;
    QString currentDirectory;
    QString currentFilename;

private slots:
    void recordingStarted();
    void recordingFinished();
    bool validateRecordInputs();
    void recordFilterShouldUpdate();

    bool startPcapRecord(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);
    bool startTsRecord(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);
    void on_recordExpandPCAPFilterButton_toggled(bool);
    void on_recordFileFormatPCAP_toggled(bool);
    void on_recordOpenFileDialog_clicked();
    void on_recordStartStopBtn_clicked();
    bool fileExists(QString);

public slots:
    void recordStatusChanged(FinalStatus status);
    void recordWorkerStatusChanged(WorkerStatus status);

};

#endif // RECORDWIDGET_H
