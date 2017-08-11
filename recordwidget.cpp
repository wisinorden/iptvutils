#include "recordwidget.h"
#include "ui_recordwidget.h"
#include "mainwindow.h"
#include "validator.h"
#include "pcapfilter.h"

#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

NetworkPcapFileRecorder* RecordWidget::networkPcapFileRecorder;
TsNetworkFileRecorder* RecordWidget::tsNetworkFileRecorder;

RecordWidget::RecordWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordWidget)
{
    ui->setupUi(this);

    connect(ui->recordHost, &QLineEdit::textChanged, this, &RecordWidget::recordFilterShouldUpdate);
    connect(ui->recordPort, &QLineEdit::textChanged, this, &RecordWidget::recordFilterShouldUpdate);
    connect(ui->recordRtpFecCheckBox, &QCheckBox::stateChanged, this, &RecordWidget::recordFilterShouldUpdate);

    for (int i = 0; i < MainWindow::interfaces.length(); i++) {
        ui->recordInterfaceSelect->addItem(MainWindow::interfaces.at(i).getName());
        ui->recordInterfaceSelect->setItemData(i, MainWindow::interfaces.at(i).getName(), Qt::ToolTipRole);
    }

    // revalidate fields and calculate filter
    validateRecordInputs();
    recordFilterShouldUpdate();
}

RecordWidget::~RecordWidget()
{
    delete ui;
}

void RecordWidget::loadSettings() {
    QSettings settings;

    settings.beginGroup("record");
    ui->recordInterfaceSelect->setCurrentIndex(settings.value("interface", 0).toInt());
    ui->recordHost->setText(settings.value("host", "238.123.123.100").toString());
    ui->recordPort->setText(settings.value("port", "1234").toString());
    ui->recordRtpFecCheckBox->setChecked(settings.value("rtp-fec", false).toBool());
    ui->recordFilename->setText(settings.value("filename", "").toString());
    settings.endGroup();
}

void RecordWidget::saveSettings() {
    QSettings settings;

    settings.beginGroup("record");
    settings.setValue("interface", ui->recordInterfaceSelect->currentIndex());
    settings.setValue("host", ui->recordHost->text());
    settings.setValue("port", ui->recordPort->text());
    settings.setValue("rtp-fec", ui->recordRtpFecCheckBox->isChecked());
    settings.setValue("filename", ui->recordFilename->text());
    settings.endGroup();
}

void RecordWidget::recordingStarted() {
    ui->recordCancelBtn->setEnabled(true);
}

void RecordWidget::recordStatusChanged(FinalStatus status) {
    ui->recordStatus->setText(status.toUiString());
}

void RecordWidget::recordWorkerStatusChanged(WorkerStatus status) {
    status.insertIntoTree(ui->treeWidget);
}

void RecordWidget::recordingFinished() {
    ui->startPcapRecordBtn->setEnabled(true);
    ui->startTsRecordBtn->setEnabled(true);
    ui->recordCancelBtn->setEnabled(false);
    networkPcapFileRecorder = NULL;
    tsNetworkFileRecorder = NULL;
}

bool RecordWidget::validateRecordInputs() {
    bool valid = true;

    // Performs validation on everything
    valid = Validator::validateIp(ui->recordHost) && valid;
    valid = Validator::validatePort(ui->recordPort) && valid;

    return valid;
}

void RecordWidget::recordFilterShouldUpdate() {
    if (validateRecordInputs()) {
        ui->recordFilter->setText(PcapFilter::generateFilter(ui->recordHost->text(), ui->recordPort->text().toShort(), ui->recordRtpFecCheckBox->checkState() == Qt::Checked));
        ui->recordFilter->setToolTip(ui->recordFilter->text());
    }
}

bool RecordWidget::startPcapRecord(WorkerConfiguration::WorkerMode mode) {
    if (networkPcapFileRecorder != NULL) {
        qWarning("Atempt to start PcapRecord while pointer not released");
        return false;
    }
    NetworkInputConfiguration inputConfig(
                MainWindow::interfaces.at(ui->recordInterfaceSelect->currentIndex()),
                ui->recordHost->text(),
                ui->recordPort->text().toShort(),
                ui->recordFilter->text());
    FileOutputConfiguration outputConfig(ui->recordFilename->text(), FileConfiguration::PCAP);
    WorkerConfiguration config(inputConfig, outputConfig, mode);
    networkPcapFileRecorder = new NetworkPcapFileRecorder(config, this);
    connect(networkPcapFileRecorder, &NetworkPcapFileRecorder::started, this, &RecordWidget::recordingStarted);
    connect(networkPcapFileRecorder, &NetworkPcapFileRecorder::finished, this, &RecordWidget::recordingFinished);
    connect(networkPcapFileRecorder, &NetworkPcapFileRecorder::status, this, &RecordWidget::recordStatusChanged);
    connect(networkPcapFileRecorder, &NetworkPcapFileRecorder::workerStatus, this, &RecordWidget::recordWorkerStatusChanged);

    ui->startPcapRecordBtn->setEnabled(false);
    ui->startTsRecordBtn->setEnabled(false);
    ui->recordCancelBtn->setEnabled(false);

    networkPcapFileRecorder->start();
    return true;
}

bool RecordWidget::startTsRecord(WorkerConfiguration::WorkerMode mode) {
    if (networkPcapFileRecorder != NULL) {
        qWarning("Atempt to start TsRecord while pointer not released");
        return false;
    }
    NetworkInputConfiguration inputConfig(
                MainWindow::interfaces.at(ui->recordInterfaceSelect->currentIndex()),
                ui->recordHost->text(),
                ui->recordPort->text().toShort(),
                ui->recordFilter->text());
    FileOutputConfiguration outputConfig(ui->recordFilename->text()+".ts", FileConfiguration::TS);
    WorkerConfiguration config(inputConfig, outputConfig, mode);

    tsNetworkFileRecorder = new TsNetworkFileRecorder(config, this);
    connect(tsNetworkFileRecorder, &TsNetworkFileRecorder::started, this, &RecordWidget::recordingStarted);
    connect(tsNetworkFileRecorder, &TsNetworkFileRecorder::finished, this, &RecordWidget::recordingFinished);
    connect(tsNetworkFileRecorder, &TsNetworkFileRecorder::status, this, &RecordWidget::recordStatusChanged);
    connect(tsNetworkFileRecorder, &TsNetworkFileRecorder::workerStatus, this, &RecordWidget::recordWorkerStatusChanged);

    ui->startPcapRecordBtn->setEnabled(false);
    ui->startTsRecordBtn->setEnabled(false);
    ui->recordCancelBtn->setEnabled(false);

    tsNetworkFileRecorder->start();
    return true;
}

void RecordWidget::on_recordOpenFileDialog_clicked()
{
    QString path = ui->recordFilename->text();
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Select save location"), path, tr("Pcap & ts files (*.pcap *.ts)"));
    if (filename != "")
        ui->recordFilename->setText(filename);
}

void RecordWidget::on_startPcapRecordBtn_clicked()
{
    if (ui->recordInterfaceSelect->currentIndex() == -1) {
        return;
    }
    if (ui->recordFilename->text().length() == 0) {
        return;
    }
    if (!validateRecordInputs()) {
        return;
    }

    startPcapRecord();
    //startPcapRecord(WorkerConfiguration::ANALYSIS_MODE_LIVE);
}

void RecordWidget::on_startTsRecordBtn_clicked()
{
    if (ui->recordInterfaceSelect->currentIndex() == -1) {
        return;
    }
    if (ui->recordFilename->text().length() == 0) {
        return;
    }
    if (!validateRecordInputs()) {
        return;
    }

    startTsRecord();
    //startTsRecord(WorkerConfiguration::ANALYSIS_MODE_LIVE);
}

void RecordWidget::on_recordCancelBtn_clicked()
{
    if (networkPcapFileRecorder != Q_NULLPTR) {
        qInfo("atempting to stop pcapRecorder");
        networkPcapFileRecorder->stop();
    }
    if (tsNetworkFileRecorder != NULL) {
        qInfo("atempting to stop tsRecorder");
        tsNetworkFileRecorder->stop();
    }
}
