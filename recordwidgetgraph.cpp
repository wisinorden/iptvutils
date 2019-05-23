#include "recordwidgetgraph.h"
#include "chart.h"

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtGui/QMouseEvent>



RecordWidgetGraph::RecordWidgetGraph( QWidget *parent):
    QChartView(parent),
    m_isTouching(false)

{
    m_isTouching = false;

    setDragMode(QGraphicsView::NoDrag);
    this->setMouseTracking(true);
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setFocusPolicy(Qt::StrongFocus);

}





QChart* RecordWidgetGraph::setupGraph(){

    lineSeries = new QLineSeries();

    // Create chart, add data, hide legend, and add axis
    Chart * chart = new Chart();

    chart->legend()->hide();
    chart->addSeries(lineSeries);
    chart->createDefaultAxes();

    // Customize the title font
    QFont font;
    font.setPixelSize(8);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush(Qt::black));
 //   chart->setTitle("Bitrate per second mbps");

    // Change the line color and weight
    QPen pen(QRgb(0x000000));
    pen.setWidth(1);
    lineSeries->setPen(pen);

    this->setChart(chart);
  //  this->chart()->axisX()->setTitleText("Seconds");

    this->chartCounter = 0;

    bitrateTimer.start();
    return(chart);

}



void RecordWidgetGraph::setBitrate (qint64 bitrate, qint64 duration){
    this->currentBitrate = bitrate;
    this->bitrateTimestamp = duration;


    if(bitrate != 0 /*&& chartCounter < 65 */){

        double doubleBit = currentBitrate/ 1000000;

        lineSeries->append(bitrateTimestamp, currentBitrate);
        this->chart()->removeSeries(lineSeries);
        this->chart()->addSeries(lineSeries);
        this->chart()->createDefaultAxes();




        this->chart()->scroll(chartCounter /5 , 0);
        this->chart()->axisX()->setRange((bitrateTimer.elapsed()) - 2000, bitrateTimer.elapsed()+500);

        this->repaint();

        chartCounter++;
    }

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
    if (m_isTouching)
        return;
    QChartView::mouseMoveEvent(event);
}

void RecordWidgetGraph::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isTouching)
        m_isTouching = false;

    // Because we disabled animations when touch event was detected
    // we must put them back on.
    chart()->setAnimationOptions(QChart::SeriesAnimations);

    QChartView::mouseReleaseEvent(event);
}

void RecordWidgetGraph::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;

    case Qt::Key_Left:
        chart()->scroll(-10, 0);

        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);

        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

