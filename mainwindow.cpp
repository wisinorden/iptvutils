#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "interface.h"
#include "igmp.h"
#include "Player/pcapfilenetworkplayer.h"
#include "Recorder/networkpcapfilerecorder.h"
#include "Recorder/tsnetworkfilerecorder.h"
#include "Recorder/pcaptsconverter.h"

#include <QSettings>

QList<Interface> MainWindow::interfaces = QList<Interface>();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Interface::getInterfaces(&interfaces);
    ui->setupUi(this);

    IGMP::init();
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings() {
    QSettings settings;

    ui->tabWidget->setCurrentIndex(settings.value("ui/selectedTab", 0).toInt());

    ui->playbackWidget->loadSettings();
    ui->recordWidget->loadSettings();
    ui->convertWidget->loadSettings();

}

void MainWindow::saveSettings() {
    QSettings settings;

    settings.setValue("ui/selectedTab", ui->tabWidget->currentIndex());

    ui->playbackWidget->saveSettings();
    ui->recordWidget->saveSettings();
    ui->convertWidget->saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qInfo("Closing workers");
    if (PlaybackWidget::pcapFileNetworkPlayer != NULL) {
        qInfo("pcapFileNetworkPlayer stopAndWait");
        PlaybackWidget::pcapFileNetworkPlayer->stopAndWait();
    }
    if (RecordWidget::RecordWidget::networkPcapFileRecorder != NULL) {
        qInfo("networkPcapFileRecorder stopAndWait");
        RecordWidget::networkPcapFileRecorder->stopAndWait();
    }
    if (RecordWidget::tsNetworkFileRecorder != NULL) {
        qInfo("tsNetworkFileRecorder stopAndWait");
        RecordWidget::tsNetworkFileRecorder->stopAndWait();
    }
    if (ConvertWidget::pcapTsConverter != NULL) {
        qInfo("pcapTsConverter stopAndWait");
        ConvertWidget::pcapTsConverter->stopAndWait();
    }
    saveSettings();
    event->accept();
}
