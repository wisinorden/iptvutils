#ifndef RECORDWIDGETGRAPH_H
#define RECORDWIDGETGRAPH_H

#include <QObject>

#include "mainwindow.h"
#include "recordwidget.h"
#include "Middleware/analyzerpcapmiddleware.h"


#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QRubberBand>


QT_CHARTS_USE_NAMESPACE

class RecordWidgetGraph : public QChartView
{


public:


    RecordWidgetGraph(QWidget *parent);
    QChart GraphChart;
    qint16 chartCounter;
    double maxBitrate;
    double minBitrate;
    QChart* setupGraph();

    void setAvgBitrate(double avgBitrate);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event) override;



protected:



public slots:
    void setBitrate (double bitrate, qint64 duration);
    void setYAxisTitle(QString title);




private:
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
