#ifndef RECORDWIDGETGRAPH_H
#define RECORDWIDGETGRAPH_H

#include <QObject>

#include "mainwindow.h"
#include <QApplication>
#include "recordwidget.h"
#include "ui_recordwidget.h"

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
    QChart GraphChart;
    RecordWidgetGraph();
    qint64 chartCounter;
    qreal x;
    QChart* setupGraph();

    void keyPressEvent(QKeyEvent *event);


protected:
  //  virtual void mousePressEvent(QMouseEvent *event) override;
  //  virtual void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void bitrateInfoUpdate (FinalStatus status);

private:
    QLineSeries *lineSeries;
    QPointF m_lastMousePos;
};
#endif // RECORDWIDGETGRAPH_H
