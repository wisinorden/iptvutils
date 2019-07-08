#ifndef RECORDWIDGETGRAPH_H
#define RECORDWIDGETGRAPH_H

#include <QObject>

#include "mainwindow.h"
#include "recordwidget.h"


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
    QChart GraphChart;
    qint16 chartCounter;
    quint8 noOfStreams;
    double maxBitrate;
    double minBitrate;
    QChart* setupGraph();
//    QChart* changeStream(int i);

    void setAvgBitrate(double avgBitrate);
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




public slots:
    void setBitrate (double bitrate, qint64 duration);
    void setYAxisTitle(QString title);




private:
    bool firstRound = true;
    QDateTimeAxis *axisX;
    quint32 zoomInt;
    double avgBitrate;
    qint64 durations;
    qint64 currentBitrate;
    qint64 bitrateTimestamp;
    QLineSeries *lineSeries;
    QLineSeries *avgSeries;
    QPointF m_lastMousePos;
    bool m_isTouching;
    bool isScrolling = false;

};
#endif // RECORDWIDGETGRAPH_H
