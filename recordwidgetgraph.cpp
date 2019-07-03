#include "recordwidgetgraph.h"
#include "chart.h"

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QChart>
#include <QtGui/QMouseEvent>



RecordWidgetGraph::RecordWidgetGraph( QWidget *parent):
    QChartView(parent),
    m_isTouching(false)
{
    setDragMode(QGraphicsView::NoDrag);
    this->setMouseTracking(true);

    this->setRubberBand(QChartView::RectangleRubberBand);

}



RecordWidgetGraph::~RecordWidgetGraph()
{
    for (auto ls : this->streamList) {
        delete ls;
    }
}


QChart* RecordWidgetGraph::setupGraph(){

    firstRound = true;
    streamList.clear();
    avgStreamList.clear();
    iatDevList.clear();

    lineSeries = new QLineSeries();
    avgSeries = new QLineSeries();

    avgSeries->setName("Avg bitrate");
    lineSeries->setName("Bitrate");

    // Create chart and add axis
    Chart * chart = new Chart();

    // chart->legend()->hide();
    chart->addSeries(lineSeries);
    chart->addSeries(avgSeries);
    chart->createDefaultAxes();

    chart->axisY()->setTitleText("Bitrate mbps");

    // Change the line color and weight
    QPen pen(QRgb(0x000000));
    pen.setWidth(1);
    lineSeries->setPen(pen);

    this->setChart(chart);

    this->chartCounter = 0;
    this->maxBitrate = 0;
    this->minBitrate = 5000;
    this->zoomInt = 0;
    this->avgBitrate = 0;

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("m:ss");
    axisX->setTickCount(10);
    axisX->setTitleText("Time m:s");
    this->chart()->setAxisX(axisX, lineSeries);
    avgSeries->attachAxis(axisX);
    //    this->axisX = axisX;

    return(chart);
}



void RecordWidgetGraph::setYAxisTitle(QString title){
    chart()->axisY()->setTitleText(title);

    if(title.contains("Std")){
        lineSeries->setName("IAT dev");
        chart()->removeSeries(avgSeries);
    }

}

void RecordWidgetGraph::setAvgBitrate(double avgBitrate){
    this->avgBitrate = avgBitrate;
}


void RecordWidgetGraph::setBitrate (double bitrate, qint64 duration){

    // Appends new values and updates graph
    if(bitrate > 1 && bitrate < 1000){
        this->durations = duration;
        lineSeries->append(duration, bitrate);


        if(avgBitrate != 0){
            avgSeries->append(duration, avgBitrate);
        }
        this->chart()->axisX()->setRange(QDateTime::fromMSecsSinceEpoch(duration - 20000), QDateTime::fromMSecsSinceEpoch(duration + 2000));

        if(bitrate < this->minBitrate){
            minBitrate = bitrate;
            this->chart()->axisY()->setRange(minBitrate - 0.5, this->maxBitrate + 0.5);
        }

        if (this->maxBitrate < bitrate) {
            this->maxBitrate = bitrate;
            this->chart()->axisY()->setRange(minBitrate - 0.5, this->maxBitrate + 0.5);
        }

        chartCounter++;
    }
}


void RecordWidgetGraph::changeStream(int selectedStream, bool isBitrateSignal){

    chart()->removeSeries(lineSeries);
    chart()->removeSeries(avgSeries);

    if(isBitrateSignal){
        lineSeries =  streamList[selectedStream];
        avgSeries = avgStreamList[selectedStream];

        chart()->addSeries(lineSeries);
        chart()->addSeries(avgSeries);

        avgSeries->setName("Avg bitrate");
        lineSeries->setName("Bitrate");
    } else {
        lineSeries = iatDevList[selectedStream];
        chart()->addSeries(lineSeries);
        lineSeries->setName("IAT dev");
    }

    QPen pen(QRgb(0x000000));
    pen.setWidth(1);
    lineSeries->setPen(pen);


    chart()->createDefaultAxes();

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("m:ss");
    axisX->setTickCount(10);
    axisX->setTitleText("Time m:ss");

    this->chart()->setAxisX(axisX, lineSeries);

    if(isBitrateSignal){
        avgSeries->attachAxis(chart()->axisX());
        chart()->axisY()->setTitleText("Bitrate mbps");
    } else {
        chart()->axisY()->setTitleText("Std IAT dev µs");
    }

    this->chart()->axisX()->setRange(QDateTime::fromMSecsSinceEpoch(durations - 20000), QDateTime::fromMSecsSinceEpoch(durations + 2000));

    selectedStreamIndex = selectedStream;
}

void RecordWidgetGraph::setNoOfStreams(quint8 noOfStreams){
    this->noOfStreams = noOfStreams;
}

void RecordWidgetGraph::recordMultipleStreams(WorkerStatus status){ // Collects data for multiple streams

    if (firstRound){
        for(int i = 0; i < status.streams.count(); i++){
            quint64 hashKey = status.streams.keys().at(i);

            this->streamList.append(new QLineSeries());
            this->avgStreamList.append(new QLineSeries());
            this->iatDevList.append(new QLineSeries());
            streamList[i]->append( status.streams[hashKey].currentTime, status.streams[hashKey].currentBitrate);
            avgStreamList[i]->append(status.streams[hashKey].currentTime, status.streams[hashKey].avgBitrate);
            iatDevList[i]->append(status.streams[hashKey].currentTime, status.streams[hashKey].iatDeviation);

            firstRound = false;
        }

    } else {
        for(int i = 0; i < status.streams.count(); i++){
            quint64 hashKey = status.streams.keys().at(i);
            streamList[i]->append( status.streams[hashKey].currentTime, status.streams[hashKey].currentBitrate);
            avgStreamList[i]->append(status.streams[hashKey].currentTime, status.streams[hashKey].avgBitrate);
            iatDevList[i]->append(status.streams[hashKey].currentTime, status.streams[hashKey].iatDeviation);

        }
    }
}
/*
void RecordWidgetGraph::setupLineSeries(){

}
*/
void RecordWidgetGraph::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
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
        this->zoomInt += 10000;
        this->chart()->axisX()->setRange(QDateTime::fromMSecsSinceEpoch((durations - 20000) - zoomInt), QDateTime::fromMSecsSinceEpoch(durations + 2000));
        this->chart()->axisY()->setRange(minBitrate - 0.5, this->maxBitrate + 0.5);
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

