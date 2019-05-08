#include "recordwidgetgraph.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtGui/QMouseEvent>


RecordWidgetGraph::RecordWidgetGraph()
{
     setDragMode(QGraphicsView::NoDrag);
     this->setMouseTracking(true);
     this->setRubberBand(QChartView::RectangleRubberBand);

}


QChart* RecordWidgetGraph::setupGraph(){


    lineSeries = new QLineSeries();

    // Create chart, add data, hide legend, and add axis
    QChart * chart = new QChart();

    chart->legend()->hide();
    chart->addSeries(lineSeries);
    chart->createDefaultAxes();

    // Customize the title font
    QFont font;
    font.setPixelSize(8);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush(Qt::black));
    chart->setTitle("Bitrate per second mbps");

    // Change the line color and weight
    QPen pen(QRgb(0x000000));
    pen.setWidth(1);
    lineSeries->setPen(pen);

    this->setChart(chart);
    this->chartCounter = 0;
    return(chart);
}




void RecordWidgetGraph::bitrateInfoUpdate (FinalStatus status){

    // Use status.bytes to calculate new bitrate for every 1000 ms
    double tempInt = status.getBitrate();
    //Appends Mbit/s

    if(tempInt != 0 /*&& chartCounter < 65 */){
        lineSeries->append(chartCounter, tempInt/ 1000000);
        this->chart()->removeSeries(lineSeries);
        this->chart()->addSeries(lineSeries);
        this->chart()->createDefaultAxes();


        this->chart()->scroll(chartCounter , 0);
        this->chart()->axisX()->setRange(chartCounter- 10, chartCounter + 10);
        this->repaint();
        chartCounter++;
    }
}


/*

void RecordWidgetGraph::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    }

void RecordWidgetGraph::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
        m_lastMousePos = event->pos();
        event->accept();
    }

    QChartView::mousePressEvent(event);
}

void RecordWidgetGraph::mouseMoveEvent(QMouseEvent *event)
{
    // pan the chart with a middle mouse drag
    if (event->buttons() & Qt::MiddleButton)
    {
        QRectF bounds = QRectF(0,0,0,0);
        for(auto series : this->chart()->series())
            bounds.united(series->bounds())

                    auto pos = this->chart()->mapToValue(event->pos()) - this->chart()->mapToValue(m_lastMousePos);

        if (this->rubberBand() == QChartView::RectangleRubberBand)
            this->chart()->zoom(bounds.translated(-dPos.x(), -dPos.y()));
        else if (this->rubberBand() == QChartView::HorizontalRubberBand)
            this->chart()->zoom(bounds.translated(-dPos.x(), 0));
        else if (this->rubberBand() == QChartView::VerticalRubberBand)
            this->chart()->zoom(bounds.translated(0, -dPos.y()));

        m_lastMousePos = event->pos();
        event->accept();
    }
*/

