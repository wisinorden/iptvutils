#include "playbackwidget.h"
#include "ui_playbackwidget.h"
#include "mainwindow.h"
#include "validator.h"
#include "Status/streamid.h"
#include "pcapfilter.h"

#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>

Player* PlaybackWidget::fileNetworkPlayer;
//PcapFileNetworkPlayer* PlaybackWidget::pcapFileNetworkPlayer;
//TsFileNetworkPlayer* PlaybackWidget::tsFileNetworkPlayer;
static QMap<QString, FileInputConfiguration::LoopType> loopOptions;
QHash<quint64, StreamInfo> streams;
StreamId selectedStreamId;

PlaybackWidget::PlaybackWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaybackWidget),
    started(false)
{
    ui->setupUi(this);

    // Advanced PCAP filter
    ui->playbackPcapFilterContainer->hide();
    connect(ui->playbackExpandPCAPFilterButton, SIGNAL(toggled(bool)), this, SLOT(on_playbackExpandPCAPFilterButton_toggled(bool)));

    connect(ui->playbackHost, &QLineEdit::textChanged, this, &PlaybackWidget::playbackInputChanged);
    connect(ui->playbackPort, &QLineEdit::textChanged, this, &PlaybackWidget::playbackInputChanged);

    for (int i = 0; i < MainWindow::interfaces.length(); i++) {
        ui->playbackInterfaceSelect->addItem(MainWindow::interfaces.at(i).getName());
        ui->playbackInterfaceSelect->setItemData(i, MainWindow::interfaces.at(i).getName(), Qt::ToolTipRole);
    }



    loopOptions.insert("Replay",             FileInputConfiguration::LOOP_SIMPLE);
    loopOptions.insert("CC offset, PCR discontinuity", FileInputConfiguration::LOOP_DISCONTINUITY_FLAG);
    loopOptions.insert("PCR Rewrite",        FileInputConfiguration::LOOP_PCR_REWRITE);
    ui->playbackLoopCombo->addItem(loopOptions.key(FileInputConfiguration::LOOP_SIMPLE));
    ui->playbackLoopCombo->setItemData(0,
            "No data modification, equivalent to pressing play over and over.", Qt::ToolTipRole);
    ui->playbackLoopCombo->addItem(loopOptions.key(FileInputConfiguration::LOOP_DISCONTINUITY_FLAG));
    ui->playbackLoopCombo->setItemData(1,
            "Offsets CC at loop and sets discontinuity flag on first PCR after loop.", Qt::ToolTipRole);
}

PlaybackWidget::~PlaybackWidget() {
    delete ui;
}

void PlaybackWidget::loadSettings() {
    QSettings settings;

    settings.beginGroup("playback");
    QString ifId = settings.value("interface").toString();
    for (int i = 0; i < MainWindow::interfaces.length(); i++) {
        if (ifId == MainWindow::interfaces.at(i).getId()) {
            ui->playbackInterfaceSelect->setCurrentIndex(i);
            break;
        }
    }
    ui->playbackHost->setText(settings.value("host", "").toString());
    ui->playbackPort->setText(settings.value("port", "").toString());
    ui->playbackFilename->setText(settings.value("filename", "").toString());
    ui->playbackLoopCheckbox->setChecked(settings.value("loopChecked", false).toBool());
    ui->playbackLoopCombo->setCurrentIndex(settings.value("loopType", 0).toInt());
    ui->playbackLoopCombo->setEnabled(ui->playbackLoopCheckbox->isChecked());
    currentFile = settings.value("filename", "").toString();
    settings.endGroup();

    //startTsPlayback(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
    //startPcapPlayback(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
    startPlayback(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
}

bool PlaybackWidget::startPlayback(WorkerConfiguration::WorkerMode mode) {
    if (currentFile.endsWith(".ts")) {
        qInfo("startTsPlayback analysis mode offline");
        return startTsPlayback(mode);
    }
    else if (currentFile.endsWith(".pcap") || currentFile.endsWith(".pcapng")) {
        qInfo("startPcapPlayBack analysis mode offline");
        return startPcapPlayback(mode);
    }

    return false;
}

void PlaybackWidget::saveSettings() {
    QSettings settings;

    settings.beginGroup("playback");
    settings.setValue("interface", MainWindow::interfaces.at(ui->playbackInterfaceSelect->currentIndex()).getId());
    settings.setValue("host", ui->playbackHost->text());
    settings.setValue("port", ui->playbackPort->text());
    settings.setValue("filename", currentFile);
    settings.setValue("loopChecked", ui->playbackLoopCheckbox->isChecked());
    settings.setValue("loopType", ui->playbackLoopCombo->currentIndex());
    settings.endGroup();
}

void PlaybackWidget::playbackStarted() {
    started = true;
    ui->playbackStartStopBtn->setText(tr("Stop"));
    ui->playbackStartStopBtn->setEnabled(true);
}

void PlaybackWidget::playbackStatusChanged(FinalStatus status) {
    ui->playbackStatus->setText(status.toUiString());
}

void PlaybackWidget::playbackWorkerStatusChanged(WorkerStatus status) {
    if (status.getType() == WorkerStatus::STATUS_ANALYZED_ENTIRE) {
        status.insertIntoTree(ui->treeWidget);
        streams = status.getStreams();
    }
}

void PlaybackWidget::playbackFinished() {
    ui->playbackStartStopBtn->setText(tr("Start"));
    ui->playbackStartStopBtn->setEnabled(true);
    fileNetworkPlayer = NULL;
    started = false;
}

bool PlaybackWidget::validatePlaybackInputs() {
    bool valid = true;

    // Performs validation on everything
    valid = Validator::validateIp(ui->playbackHost, true) && valid;
    valid = Validator::validatePort(ui->playbackPort, true) && valid;

    return valid;
}

void PlaybackWidget::playbackInputChanged() {
    validatePlaybackInputs();
}

bool PlaybackWidget::startTsPlayback(WorkerConfiguration::WorkerMode mode) {
    qInfo("startTsPlayback()");
    if (ui->playbackFilename->text().length() == 0)
        return false;

    if (fileNetworkPlayer != NULL) {
        qWarning("Atempt to start TsPlayback while pointer not released");
        return false;
    }

    int rewriteFlags = 0;
    QString host = ui->playbackHost->text();
    if (host.length() > 0)
        rewriteFlags |= NetworkOutputConfiguration::REWRITE_DST_HOST;

    quint16 port = ui->playbackPort->text().toUShort();
    if (ui->playbackPort->text().length() > 0)
        rewriteFlags |= NetworkOutputConfiguration::REWRITE_DST_PORT;

    FileInputConfiguration inputConfig(
                ui->playbackFilename->text(),
                FileConfiguration::TS,
                mode == WorkerConfiguration::WorkerMode::ANALYSIS_MODE_OFFLINE ?
                    "":
                    ui->playbackFilter->text(),
                ui->playbackLoopCheckbox->isChecked() ?
                    loopOptions.value(ui->playbackLoopCombo->currentText()) :
                    FileInputConfiguration::LOOP_NONE);
    NetworkOutputConfiguration outputConfig(
                MainWindow::interfaces.at(ui->playbackInterfaceSelect->currentIndex()),
                rewriteFlags,
                host,
                port);
    WorkerConfiguration config(inputConfig, outputConfig, mode);

    fileNetworkPlayer = new TsFileNetworkPlayer(config, this);
    connect(fileNetworkPlayer, &TsFileNetworkPlayer::started, this, &PlaybackWidget::playbackStarted);
    connect(fileNetworkPlayer, &TsFileNetworkPlayer::finished, this, &PlaybackWidget::playbackFinished);
    connect(fileNetworkPlayer, &TsFileNetworkPlayer::status, this, &PlaybackWidget::playbackStatusChanged);
    connect(fileNetworkPlayer, &TsFileNetworkPlayer::workerStatus, this, &PlaybackWidget::playbackWorkerStatusChanged);

    ui->playbackStartStopBtn->setEnabled(false);

    fileNetworkPlayer->start();
    return true;
}

bool PlaybackWidget::startPcapPlayback(WorkerConfiguration::WorkerMode mode) {
    qInfo("startPcapPlayback()");
    if (ui->playbackFilename->text().length() == 0)
        return false;

    if (fileNetworkPlayer != NULL) {
        qWarning("Atempt to start PcapPlayback while pointer not released");
        return false;
    }

    int rewriteFlags = 0;
    QString host = ui->playbackHost->text();
    if (host.length() > 0)
        rewriteFlags |= NetworkOutputConfiguration::REWRITE_DST_HOST;

    quint16 port = ui->playbackPort->text().toUShort();
    if (ui->playbackPort->text().length() > 0)
        rewriteFlags |= NetworkOutputConfiguration::REWRITE_DST_PORT;

    FileInputConfiguration inputConfig(
                ui->playbackFilename->text(),
                FileConfiguration::PCAP,
                mode == WorkerConfiguration::WorkerMode::ANALYSIS_MODE_OFFLINE ?
                    "":
                    ui->playbackFilter->text(),
                ui->playbackLoopCheckbox->isChecked() ?
                    loopOptions.value(ui->playbackLoopCombo->currentText()) :
                    FileInputConfiguration::LOOP_NONE);
    NetworkOutputConfiguration outputConfig(
                MainWindow::interfaces.at(ui->playbackInterfaceSelect->currentIndex()),
                rewriteFlags,
                host,
                port);
    WorkerConfiguration config(inputConfig, outputConfig, mode);


    fileNetworkPlayer = new PcapFileNetworkPlayer(config, this);
    connect(fileNetworkPlayer, &PcapFileNetworkPlayer::started, this, &PlaybackWidget::playbackStarted);
    connect(fileNetworkPlayer, &PcapFileNetworkPlayer::finished, this, &PlaybackWidget::playbackFinished);
    connect(fileNetworkPlayer, &PcapFileNetworkPlayer::status, this, &PlaybackWidget::playbackStatusChanged);
    connect(fileNetworkPlayer, &PcapFileNetworkPlayer::workerStatus, this, &PlaybackWidget::playbackWorkerStatusChanged);

    ui->playbackStartStopBtn->setEnabled(false);

    fileNetworkPlayer->start();
    return true;
}

void PlaybackWidget::on_playbackExpandPCAPFilterButton_toggled(bool checked)
{
    if (checked) {
        ui->playbackExpandPCAPFilterButton->setArrowType(Qt::ArrowType::DownArrow);
        ui->playbackPcapFilterContainer->show();
    }
    else {
        ui->playbackExpandPCAPFilterButton->setArrowType(Qt::ArrowType::RightArrow);
        ui->playbackPcapFilterContainer->hide();
    }
}

void PlaybackWidget::on_playbackOpenFileDialog_clicked() {
    QString path = currentFile;
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open recorded traffic"), path, tr("Pcap files (*.pcap *.pcapng *.ts)"));
    if (filename != "") {
        ui->playbackFilename->setText(filename);
        currentFile = filename;
    }

    startPlayback(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
}

void PlaybackWidget::on_playbackLoopCheckbox_clicked() {
    ui->playbackLoopCombo->setEnabled(ui->playbackLoopCheckbox->isChecked());
}

void PlaybackWidget::on_playbackStartStopBtn_clicked() {
    if (!started) {
        if (ui->playbackInterfaceSelect->currentIndex() == -1) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("No network interface selected!"));
            return;
        }

        if (currentFile.length() == 0) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("You must choose a file to play!"));
            return;
        }

        if (!validatePlaybackInputs()) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("The multicast address or port you have entered is invalid!"));
            return;
        }

        startPlayback();
    }
    else {
        if (fileNetworkPlayer != NULL) {
            qInfo("stopping fileNetworkPlayer");
            fileNetworkPlayer->stop();
        }
    }
}

void PlaybackWidget::on_treeWidget_itemSelectionChanged()
{
    if (ui->treeWidget->selectedItems().size() == 1 && ui->treeWidget->selectedItems().at(0)->parent() == NULL) {
        selectedStreamId = StreamId(StreamId::calcId(ui->treeWidget->selectedItems().at(0)->text(0)));
        ui->playbackFilter->setText(
            PcapFilter::generateFilter(selectedStreamId.getHost(), selectedStreamId.getPort(), false));
    }
}
