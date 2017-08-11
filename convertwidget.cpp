#include "convertwidget.h"
#include "ui_convertwidget.h"
#include "pcapfilter.h"
#include "Status/streaminfo.h"
#include "Status/streamid.h"

#include <QHash>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

PcapTsConverter* ConvertWidget::pcapTsConverter;

ConvertWidget::ConvertWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConvertWidget)
{
    ui->setupUi(this);
}

ConvertWidget::~ConvertWidget()
{
    delete ui;
}

void ConvertWidget::loadSettings() {
    QSettings settings;

    settings.beginGroup("convert");
    //ui->convertFromFilename->setText(settings.value("fromFilename", "").toString());
    ui->convertToFilename->setText(settings.value("toFilename", "").toString());
    settings.endGroup();
}

void ConvertWidget::saveSettings() {
    QSettings settings;

    settings.beginGroup("convert");
    //settings.setValue("fromFilename", ui->convertFromFilename->text());
    settings.setValue("toFilename", ui->convertToFilename->text());
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
                ui->convertFromFilename->text(),
                FileConfiguration::PCAP,
                mode == WorkerConfiguration::WorkerMode::ANALYSIS_MODE_OFFLINE ?
                    "":
                    ui->convertFilter->text());
    FileOutputConfiguration outputConfig(
                ui->convertToFilename->text(),
                FileConfiguration::TS);
    WorkerConfiguration config(inputConfig, outputConfig, mode);

    pcapTsConverter = new PcapTsConverter(config, this);
    connect(pcapTsConverter, &PcapTsConverter::finished, this, &ConvertWidget::convertFinished);
    connect(pcapTsConverter, &PcapTsConverter::status, this, &ConvertWidget::convertStatusChanged);
    connect(pcapTsConverter, &PcapTsConverter::workerStatus, this, &ConvertWidget::convertWorkerStatusChanged);
    pcapTsConverter->start();

    ui->convertStartBtn->setEnabled(false);
}

void ConvertWidget::on_OpenConvertInputDialog_clicked() {
    QString path = ui->convertFromFilename->text();
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open recorded trafic"), path, tr("Pcap files (*.pcap *pcapng)"));
    if (filename != "")
        ui->convertFromFilename->setText(filename);

    startConvert(WorkerConfiguration::ANALYSIS_MODE_OFFLINE);
}

void ConvertWidget::on_OpenConvertOutputDialog_clicked() {
    QString path = ui->convertToFilename->text();
    if (path.length() == 0)
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Select save location"), path, tr("ts files (*.ts)"));
    if (filename != "")
        ui->convertToFilename->setText(filename);
}

void ConvertWidget::on_convertStartBtn_clicked() {
    if (ui->convertFromFilename->text().length() == 0) {
        return;
    }
    if (ui->convertToFilename->text().length() == 0) {
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
