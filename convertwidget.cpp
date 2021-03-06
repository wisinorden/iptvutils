#include "convertwidget.h"
#include "ui_convertwidget.h"
#include "pcapfilter.h"
#include "Status/streaminfo.h"
#include "Status/streamid.h"

#include <QHash>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>

PcapTsConverter* ConvertWidget::pcapTsConverter;

ConvertWidget::ConvertWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConvertWidget)
{
    ui->setupUi(this);

    // Advanced PCAP filter
    ui->convertPcapFilterContainer->hide();
    connect(ui->convertExpandPCAPFilterButton, SIGNAL(toggled(bool)), this, SLOT(on_convertExpandPCAPFilterButton_toggled(bool)));
}

ConvertWidget::~ConvertWidget()
{
    delete ui;
}

void ConvertWidget::loadSettings() {
    QSettings settings;

    settings.beginGroup("convert");
    ui->convertFromFilename->setText(settings.value("fromFilename", "").toString());
    currentFromFilename = settings.value("fromFilename", "").toString();
    currentToDirectory = settings.value("toDirectory", "").toString();
    settings.endGroup();
}

void ConvertWidget::saveSettings() {
    QSettings settings;

    settings.beginGroup("convert");
    settings.setValue("fromFilename", currentFromFilename);
    settings.setValue("toFilename", currentToFilename);
    settings.setValue("toDirectory", currentToDirectory);
    settings.endGroup();
}

void ConvertWidget::convertFinished() {
    ui->convertStartBtn->setEnabled(true);
    pcapTsConverter = NULL;
}

void ConvertWidget::convertStatusChanged(FinalStatus status) {
    ui->convertStatus->setText(status.toUiString());
}

void ConvertWidget::convertWorkerStatusChanged(WorkerStatus status) {
    if (status.getType() == WorkerStatus::STATUS_ANALYZED_ENTIRE)
        status.insertIntoTree(ui->treeWidget);
}

void ConvertWidget::startConvert(WorkerConfiguration::WorkerMode mode) {
    FileInputConfiguration inputConfig(
                currentFromFilename,
                FileConfiguration::PCAP,
                mode == WorkerConfiguration::WorkerMode::ANALYSIS_MODE_OFFLINE ?
                    "":
                    ui->convertFilter->text());
    FileOutputConfiguration outputConfig(
                currentToFilename,
                FileConfiguration::TS);
    WorkerConfiguration config(inputConfig, outputConfig, mode);

    pcapTsConverter = new PcapTsConverter(config, this);
    connect(pcapTsConverter, &PcapTsConverter::finished, this, &ConvertWidget::convertFinished);
    connect(pcapTsConverter, &PcapTsConverter::status, this, &ConvertWidget::convertStatusChanged);
    connect(pcapTsConverter, &PcapTsConverter::workerStatus, this, &ConvertWidget::convertWorkerStatusChanged);
    pcapTsConverter->start();

    ui->convertStartBtn->setEnabled(false);
}

void ConvertWidget::on_convertExpandPCAPFilterButton_toggled(bool checked)
{
    if (checked) {
        ui->convertExpandPCAPFilterButton->setArrowType(Qt::ArrowType::DownArrow);
        ui->convertPcapFilterContainer->show();
    }
    else {
        ui->convertExpandPCAPFilterButton->setArrowType(Qt::ArrowType::RightArrow);
        ui->convertPcapFilterContainer->hide();
    }
}

void ConvertWidget::on_convertFromFileDialog_clicked() {
    QString path = currentFromFilename;
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open recording"), path, tr("Pcap files (*.pcap *.pcapng)"));
    if (filename != "") {
        ui->convertFromFilename->setText(filename);
        currentFromFilename = filename;
    }

    startConvert(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
}

void ConvertWidget::on_convertToFileDialog_clicked() {
    QString path = currentToDirectory;
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite);
    fileDialog.setNameFilter(tr("MPEG-TS (*.ts)"));
    fileDialog.setDefaultSuffix("ts");
    fileDialog.exec();
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = fileDialog.selectedFiles().first();
    if (filename != "" && !QFileInfo(filename).isDir()) {
        ui->convertToFilename->setText(filename);
        currentToFilename = filename;
        currentToDirectory = QFileInfo(filename).absolutePath();
    }
}

void ConvertWidget::on_convertStartBtn_clicked() {
    if (currentFromFilename.length() == 0) {
        QMessageBox::warning(
                    this,
                    tr("IPTV Utilities"),
                    tr("You must choose an input PCAP file!"));
        return;
    }

    if (currentToFilename.length() == 0) {
        QMessageBox::warning(
                    this,
                    tr("IPTV Utilities"),
                    tr("You must choose an output TS file!"));
        return;
    }

    if (QFileInfo(ui->convertFromFilename->text()).suffix() != "pcap") {
        QMessageBox::warning(
                    this,
                    tr("IPTV Utilities"),
                    tr("The input file does not have the correct suffix!"));
        return;
    }

    if (QFileInfo(ui->convertToFilename->text()).suffix() != "ts") {
        QMessageBox::warning(
                    this,
                    tr("IPTV Utilities"),
                    tr("The output file does not have the correct suffix!"));
        return;
    }

    if (QFileInfo(ui->convertToFilename->text()).exists() &&
            QFileInfo(ui->convertToFilename->text()).isFile()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Overwrite file?"), tr("The file ") + currentToFilename +
                                      tr(" already exists. Do you want to overwrite it?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    startConvert();
}

void ConvertWidget::on_treeWidget_itemSelectionChanged()
{
    if (ui->treeWidget->selectedItems().size() == 1 && ui->treeWidget->selectedItems().at(0)->parent() == NULL) {
        selectedStreamId = StreamId(StreamId::calcId(ui->treeWidget->selectedItems().at(0)->text(0)));
        ui->convertFilter->setText(
            PcapFilter::generateFilter(selectedStreamId.getHost(), selectedStreamId.getPort(), false));
    }
}
