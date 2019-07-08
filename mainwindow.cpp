#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "interface.h"
#include "igmp.h"
#include <QtGui/QMouseEvent>
#include "Player/pcapfilenetworkplayer.h"
#include "Recorder/networkpcapfilerecorder.h"
#include "Recorder/tsnetworkfilerecorder.h"
#include "Recorder/pcaptsconverter.h"

#include <QMessageBox>
#include <QSettings>
QList<Interface> MainWindow::interfaces = QList<Interface>();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Interface::getInterfaces(&interfaces);
    ui->setupUi(this);


    // Setup strings
    setWindowTitle(QCoreApplication::applicationName());
    ui->actionAbout->setText(tr("About %1...").arg(QCoreApplication::applicationName()));

    IGMP::init();
    loadSettings();
    this->setMouseTracking(true);
   // this->setRubberBand(QChartView::RectangleRubberBand);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings() {
    ui->tabWidget->setCurrentIndex(0);
    ui->playbackWidget->loadSettings();
    ui->recordWidget->loadSettings();
    ui->convertWidget->loadSettings();
    //ui->recordWidget->graph.setFocus();



}

void MainWindow::saveSettings() {
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{

    ui->recordWidget->graph.keyPressEvent(event);
    QMainWindow::keyPressEvent(event);

}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
 //   ui->recordWidget->graph.mousePressEvent(event);


  //  QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    ui->recordWidget->graph.mouseMoveEvent(event);

    QMainWindow::mouseMoveEvent(event);
}




void MainWindow::on_actionExit_triggered()
{
    close();
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About %1").arg(QCoreApplication::applicationName()),
                           tr("<h2>%1 %2</h2>Based on Qt %3 (%4)<br /><br />"
                              "Copyright 2017 WISI Norden AB. All rights reserved.<br /><br />"
                              "%1 is free software: you can redistribute it and/or modify "
                              "it under the terms of the GNU General Public License as published by "
                              "the Free Software Foundation, either version 3 of the License, or "
                              "(at your option) any later version.<br />"
                              "<br />"
                              "This program is distributed in the hope that it will be useful, "
                              "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                              "GNU General Public License for more details."
                              "<br />"
                              "You should have received a copy of the GNU General Public License "
                              "along with this program.  If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
                              "<br /><br />Website: <a href=\"https://github.com/wisinorden/iptvutils\">https://github.com/wisinorden/iptvutils</a>")
                       .arg(QCoreApplication::applicationName())
                       .arg(QCoreApplication::applicationVersion())
                       .arg(QString(qVersion()))
                       .arg(QSysInfo::buildAbi()));
}
