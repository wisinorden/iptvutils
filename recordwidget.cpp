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
    ui(new Ui::RecordWidget),
    started(false)
{
    ui->setupUi(this);

    // Advanced PCAP filter
    ui->recordPcapFilterContainer->hide();
    connect(ui->recordExpandPCAPFilterButton, SIGNAL(toggled(bool)), this, SLOT(on_recordExpandPCAPFilterButton_toggled(bool)));

    // File format
    connect(ui->recordFileFormatPCAP, SIGNAL(toggled(bool)), this, SLOT(on_recordFileFormatPCAP_toggled(bool)));

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
    ui->recordHost->setText(settings.value("host", "").toString());
    ui->recordPort->setText(settings.value("port", "").toString());
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
    started = true;
    ui->recordStartStopBtn->setText(tr("Stop recording"));
    ui->recordStartStopBtn->setEnabled(true);
}

void RecordWidget::recordStatusChanged(FinalStatus status) {
    ui->recordStatus->setText(status.toUiString());
}

void RecordWidget::recordWorkerStatusChanged(WorkerStatus status) {
    status.insertIntoTree(ui->treeWidget);
}

void RecordWidget::recordingFinished() {
    ui->recordStartStopBtn->setText(tr("Start recording"));
    ui->recordStartStopBtn->setEnabled(true);
    networkPcapFileRecorder = NULL;
    tsNetworkFileRecorder = NULL;
    started = false;
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

    ui->recordStartStopBtn->setEnabled(false);

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

    ui->recordStartStopBtn->setEnabled(false);

    tsNetworkFileRecorder->start();
    return true;
}

void RecordWidget::on_recordExpandPCAPFilterButton_toggled(bool checked)
{
    if (checked) {
        ui->recordExpandPCAPFilterButton->setArrowType(Qt::ArrowType::DownArrow);
        ui->recordPcapFilterContainer->show();
    }
    else {
        ui->recordExpandPCAPFilterButton->setArrowType(Qt::ArrowType::RightArrow);
        ui->recordPcapFilterContainer->hide();
    }
}

void RecordWidget::on_recordFileFormatPCAP_toggled(bool checked)
{
    QString newExtension = checked ? "pcap" : "ts";

    if (ui->recordFilename->text().length() > 0) {
        ui->recordFilename->setText(QString("%1.%2").arg(ui->recordFilename->text().left(ui->recordFilename->text().lastIndexOf("."))).arg(newExtension));
    }
}

void RecordWidget::on_recordOpenFileDialog_clicked()
{
    QString path = ui->recordFilename->text();
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString fileFilter = ui->recordFileFormatPCAP->isChecked() ? tr("Capture file (*.pcap)") : tr("MPEG-TS (*.ts)");

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Select save location"), path, fileFilter);
    if (filename != "")
        ui->recordFilename->setText(filename);
}

void RecordWidget::on_recordStartStopBtn_clicked()
{
    if (!started) {
        if (ui->recordInterfaceSelect->currentIndex() == -1) {
            return;
        }
        if (ui->recordFilename->text().length() == 0) {
            return;
        }
        if (!validateRecordInputs()) {
            return;
        }

        if (ui->recordFileFormatPCAP->isChecked()) {
            startPcapRecord();
            //startPcapRecord(WorkerConfiguration::ANALYSIS_MODE_LIVE);
        }
        else {
            startTsRecord();
            //startTsRecord(WorkerConfiguration::ANALYSIS_MODE_LIVE);
        }
    }
    else {
        if (networkPcapFileRecorder != Q_NULLPTR) {
            qInfo("atempting to stop pcapRecorder");
            ui->recordStartStopBtn->setEnabled(false);
            networkPcapFileRecorder->stop();
        }
        if (tsNetworkFileRecorder != NULL) {
            qInfo("atempting to stop tsRecorder");
            ui->recordStartStopBtn->setEnabled(false);
            tsNetworkFileRecorder->stop();
        }
    }
}
