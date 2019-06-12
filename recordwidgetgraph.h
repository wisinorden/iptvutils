#ifndef RECORDWIDGETGRAPH_H
#define RECORDWIDGETGRAPH_H

#include <QObject>

#include "mainwindow.h"
#include <QApplication>
#include "recordwidget.h"
#include "ui_recordwidget.h"
#include "Middleware/analyzerpcapmiddleware.h"
#include <QElapsedTimer>



// Manages the applications main settings like
// widget initialization
#include <QApplication>

// The main window to which you add toolbars,
// menubars, widgets and status bar
#include <QtWidgets/QMainWindow>

// Widget used to display charts
#include <QtCharts/QChartView>

// Used to create a line chart
#include <QtCharts/QLineSeries>

// Used to change names on axis
#include <QtCharts/QCategoryAxis>

#include <QtGui/QMouseEvent>
#include <QtWidgets/QRubberBand>


QT_CHARTS_USE_NAMESPACE

class RecordWidgetGraph : public QChartView
{


public:


    RecordWidgetGraph(QWidget *parent);
    QChart GraphChart;
    qint64 chartCounter;
    double maxBitrate;
    double minBitrate;
    QChart* setupGraph();

    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event) override;



protected:


public slots:
    void setBitrate (double bitrate, qint64 duration);



private:
    QElapsedTimer bitrateTimer;
    qint64 scrollCounter = 0;
    qint64 currentBitrate;
    qint64 bitrateTimestamp;
    QLineSeries *lineSeries;
    QPointF m_lastMousePos;
    bool m_isTouching;
    bool isScrolling = false;
    bool scrollCountIsSet =false;

};
#endif // RECORDWIDGETGRAPH_H
