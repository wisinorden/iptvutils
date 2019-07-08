#ifndef PLAYBACKWIDGET_H
#define PLAYBACKWIDGET_H

#include "Configuration/workerconfiguration.h"
#include "Player/pcapfilenetworkplayer.h"
#include <QWidget>
#include <recordwidget.h>
#include <recordwidgetgraph.h>

namespace Ui {
class PlaybackWidget;
}

class PlaybackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackWidget(QWidget *parent = 0);
    ~PlaybackWidget();

    void loadSettings();
    void saveSettings();

    static PcapFileNetworkPlayer *pcapFileNetworkPlayer;

private:
    Ui::PlaybackWidget *ui;
    RecordWidgetGraph graph;
    bool started;
    bool validatePlaybackInputs();
    bool startPcapPlayback(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);
    QString currentFile;

private slots:
    void playbackInputChanged();
    void on_playbackExpandPCAPFilterButton_toggled(bool);
    void on_playbackOpenFileDialog_clicked();
    void on_playbackLoopCheckbox_clicked();
    void on_playbackStartStopBtn_clicked();

    void on_treeWidget_itemSelectionChanged();

public slots:
    void playbackStarted();
    void playbackStatusChanged(FinalStatus status);
    void playbackWorkerStatusChanged(WorkerStatus status);
    void playbackFinished();
};

#endif // PLAYBACKWIDGET_H
