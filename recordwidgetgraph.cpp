#include "recordwidgetgraph.h"

RecordWidgetGraph::RecordWidgetGraph()
{

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
    chart->setTitle("Bitrate per second");

    // Change the line color and weight
    QPen pen(QRgb(0x000000));
    pen.setWidth(1);
    lineSeries->setPen(pen);


    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 10);
    chart->setAxisY(axisY);

    this->setChart(chart);
    this->chartCounter = 0;
    return(chart);
}




void RecordWidgetGraph::bitrateInfoUpdate (FinalStatus status){

    double tempInt = status.getBitrate();
    //Appends Mbit/s

    if(tempInt != 0 /*&& chartCounter < 65 */){
    lineSeries->append(chartCounter, tempInt/ 1000000);
    this->chart()->removeSeries(lineSeries);
    this->chart()->addSeries(lineSeries);
    this->chart()->createDefaultAxes();

    this->repaint();
    chartCounter++;
    }
}



