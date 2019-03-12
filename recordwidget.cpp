#include "recordwidget.h"
#include "ui_recordwidget.h"
#include "mainwindow.h"
#include "validator.h"
#include "pcapfilter.h"

#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>

#include <iostream>

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
    connect(ui-> recordFileFormatTS, SIGNAL(toggled(bool)), this, SLOT(on_recordFileFormatTS_toggled(bool)));

    connect(ui->recordHost, &QLineEdit::textChanged, this, &RecordWidget::recordFilterShouldUpdate);
    connect(ui->recordPort, &QLineEdit::textChanged, this, &RecordWidget::recordFilterShouldUpdate);
    connect(ui->recordRtpFecCheckBox, &QCheckBox::stateChanged, this, &RecordWidget::recordFilterShouldUpdate);
    connect(ui->recordUnicastCheckBox, &QCheckBox::stateChanged, this, &RecordWidget::recordFilterShouldUpdate);

    for (int i = 0; i < MainWindow::interfaces.length(); i++) {
        ui->recordInterfaceSelect->addItem(MainWindow::interfaces.at(i).getName());
        ui->recordInterfaceSelect->setItemData(i, MainWindow::interfaces.at(i).getName(), Qt::ToolTipRole);
    }

    // revalidate fields and calculate filter
    validateAdressInputs();
    validatePortInputs();
    recordFilterShouldUpdate();
}

RecordWidget::~RecordWidget()
{
    delete ui;
}

void RecordWidget::loadSettings() {
    QSettings settings;

    settings.beginGroup("record");
    QString ifId = settings.value("interface").toString();
    for (int i = 0; i < MainWindow::interfaces.length(); i++) {
        if (ifId == MainWindow::interfaces.at(i).getId()) {
            ui->recordInterfaceSelect->setCurrentIndex(i);
            break;
        }
    }
    ui->recordHost->setText(settings.value("host", "").toString());
    ui->recordPort->setText(settings.value("port", "").toString());
    ui->recordRtpFecCheckBox->setChecked(settings.value("rtp-fec", false).toBool());
    ui->recordUnicastCheckBox->setChecked(settings.value("unicast", false).toBool());
    currentDirectory = settings.value("directory", "").toString();
    settings.endGroup();
}

void RecordWidget::saveSettings() {
    QSettings settings;

    settings.beginGroup("record");
    settings.setValue("interface", MainWindow::interfaces.at(ui->recordInterfaceSelect->currentIndex()).getId());
    settings.setValue("host", ui->recordHost->text());
    settings.setValue("port", ui->recordPort->text());
    settings.setValue("rtp-fec", ui->recordRtpFecCheckBox->isChecked());
    settings.setValue("unicast", ui->recordUnicastCheckBox->isChecked());
    settings.setValue("filename", currentFilename);
    settings.setValue("directory", currentDirectory);
    settings.endGroup();
}

void RecordWidget::recordingStarted() {
    started = true;
    ui->recordStartStopBtn->setText(tr("Stop recording"));
    ui->recordOpenFileDialog->setEnabled(false);
    ui->recordStartStopBtn->setEnabled(true);
    ui->recordHost->setEnabled(false);
    ui->recordPort->setEnabled(false);
    ui->recordRtpFecCheckBox->setEnabled(false);
    ui->recordUnicastCheckBox->setEnabled(false);
    ui->recordExpandPCAPFilterButton->setEnabled(false);
    ui->recordPcapFilterContainer->setEnabled(false);
    ui->recordFileFormatPCAP->setEnabled(false);
    ui-> recordFileFormatTS->setEnabled(false);
 //   ui->recordExpandPCAPFilterButton->setEnabled(false);



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
    ui->recordOpenFileDialog->setEnabled(true);
    ui->recordHost->setEnabled(true);
    ui->recordPort->setEnabled(true);
    ui->recordRtpFecCheckBox->setEnabled(true);
    ui->recordUnicastCheckBox->setEnabled(true);
    ui->recordExpandPCAPFilterButton->setEnabled(true);
    ui->recordPcapFilterContainer->setEnabled(true);
    ui->recordFileFormatPCAP->setEnabled(true);
    ui-> recordFileFormatTS->setEnabled(true);

    networkPcapFileRecorder = NULL;
    tsNetworkFileRecorder = NULL;
    started = false;
}


bool RecordWidget::validatePortInputs() {
    bool valid = true;

    // Performs validation on Port and sets correct color

    valid = Validator::validatePort(ui->recordPort) && valid;

    return valid;
}



bool RecordWidget::validateAdressInputs() {
    bool valid = true;

    // Performs validation on Address and sets correct color
    valid = Validator::validateIp(ui->recordHost) && valid;

    return valid;
}



void RecordWidget::recordFilterShouldUpdate() {
    QHostAddress address;

    if (ui->recordUnicastCheckBox->checkState() == Qt::Checked) {
        // Use address of selected interface if unicast. If Unicastbox is checked forbids user to write in adress field
        address = MainWindow::interfaces.at(ui->recordInterfaceSelect->currentIndex()).getAddress();
        ui->recordFilter->setText(PcapFilter::generateFilter(address.toString(), ui->recordPort->text().toUShort(), ui->recordRtpFecCheckBox->checkState() == Qt::Checked));
        ui->recordHost->setEnabled(false);

    }
    else {
        ui->recordFilter->setText(PcapFilter::generateFilter(ui->recordHost->text(), ui->recordPort->text().toUShort(), ui->recordRtpFecCheckBox->checkState() == Qt::Checked));
        ui->recordHost->setEnabled(true);
    }
    if(validateAdressInputs()){
        //Sets color of address field

        ui->recordHost->setStyleSheet("");

    } else {
        ui->recordHost->setStyleSheet("QLineEdit{background: #ffd3d3;}");

    }
      //Sets color of port field
    if(validatePortInputs()){

        ui->recordPort->setStyleSheet("");

    } else {
        ui->recordPort->setStyleSheet("QLineEdit{background: #ffd3d3;}");

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
    FileOutputConfiguration outputConfig(currentFilename, FileConfiguration::PCAP);
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
    FileOutputConfiguration outputConfig(currentFilename, FileConfiguration::TS);
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
    QString path = currentDirectory;
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite);
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if (ui->recordFileFormatPCAP->isChecked()) {
        fileDialog.setNameFilter(tr("Capture file (*.pcap)"));
        fileDialog.setDefaultSuffix("pcap");
        fileDialog.exec();
    }


    else {
        fileDialog.setNameFilter(tr("MPEG-TS (*.ts)"));
        fileDialog.setDefaultSuffix("ts");
        fileDialog.exec();
    }

    if (!fileDialog.selectedFiles().empty()){
        QString filename = fileDialog.selectedFiles().first();
        if (filename != "" && !QFileInfo(filename).isDir()) {
            ui->recordFilename->setText(filename);
            currentFilename = filename;
            currentDirectory = QFileInfo(filename).absolutePath();
        }
    }
}

void RecordWidget::on_recordStartStopBtn_clicked()
{
    if (!started) {
        if (ui->recordInterfaceSelect->currentIndex() == -1) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("No network interface selected!"));
            return;
        }

        if (ui->recordFilename->text().length() == 0) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("You must choose an output file!"));
            return;
        }

        if (ui->recordFilename->text() > 0) {
            QString suffix = ui->recordFileFormatPCAP->isChecked() ? "pcap" : "ts";
            if (QFileInfo(ui->recordFilename->text()).suffix() != suffix) {
                QMessageBox::warning(
                            this,
                            tr("IPTV Utilities"),
                            tr("The output file does not have the correct suffix!"));
                return;
            }
        }

        if (!validatePortInputs() || !validateAdressInputs()) {
            QMessageBox::warning(
                        this,
                        tr("IPTV Utilities"),
                        tr("The multicast address or port you have entered is invalid!"));
            return;
        }

        if (QFileInfo(ui->recordFilename->text()).exists() &&
                QFileInfo(ui->recordFilename->text()).isFile()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Overwrite file?"), tr("The file ") + currentFilename +
                                          tr(" already exists. Do you want to overwrite it?"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return;
        }

        if (ui->recordFileFormatPCAP->isChecked()) {
            startPcapRecord();
        }
        else {
            startTsRecord();
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
