#ifndef RECORDWIDGETGRAPH_H
#define RECORDWIDGETGRAPH_H

#include <QObject>

#include "mainwindow.h"
#include "recordwidget.h"
#include "recordtxtprinter.h"


#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QRubberBand>


QT_CHARTS_USE_NAMESPACE

class RecordWidgetGraph : public QChartView
{

public:

    quint8 selectedStreamIndex;

    RecordWidgetGraph(QWidget *parent);
    ~RecordWidgetGraph();
    RecordTxtPrinter *printer;
    QChart GraphChart;
    qint16 chartCounter;
    quint8 noOfStreams;
    double maxBitrate;
    double minBitrate;
    double maxIatDev;
    double minIatDev;
    QChart* setupGraph();

    void setAvgBitrate(double avgBitrate);
    void setCurrentFileName(QString string);
    void changeStream(int i, bool isBitrateSignal);
    void recordMultipleStreams(WorkerStatus status);
    void setNoOfStreams(quint8 streams);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event) override;



protected:
    QList <QLineSeries *> streamList;
    QList <QLineSeries *> avgStreamList;
    QList <QLineSeries *> iatDevList;
    QList <QFile *> fileList;




public slots:
    void setBitrate (double bitrate, qint64 duration, bool isBitrateSignal);
    void setYAxisTitle(QString title);




private:

    QString currentFileName;
    QDateTimeAxis *axisX;
    QString filename="/home/marko.marinkovic/Documents/Data.txt";
    QFile txtFile;
    quint32 zoomInt;
    double avgBitrate;
    qint64 durations;
    qint64 currentBitrate;
    qint64 bitrateTimestamp;
    quint64 dataRefreshCounter;
    QLineSeries *lineSeries;
    QLineSeries *avgSeries;
    QPointF m_lastMousePos;
    bool m_isTouching;
    bool isScrolling = false;

    void refreshData(WorkerStatus status);
    void setAxisRange(bool isBitrateSignal, double bitrate);


};
#endif // RECORDWIDGETGRAPH_H
