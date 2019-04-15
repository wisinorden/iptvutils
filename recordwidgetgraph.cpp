#include "recordwidgetgraph.h"

RecordWidgetGraph::RecordWidgetGraph()
{

}


QChart* RecordWidgetGraph::setupGraph(){


    lineSeries = new QLineSeries();

    lineSeries->append(0, 16);
    lineSeries->append(1, 0);
    lineSeries->append(2, 24);
    lineSeries->append(3, 19);
    lineSeries->append(4, 33);
    lineSeries->append(5, 25);
    lineSeries->append(6, 34);




    // Create chart, add data, hide legend, and add axis
    QChart * chart = new QChart();

    chart->setAnimationDuration(0);
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
    pen.setWidth(5);
    lineSeries->setPen(pen);

    chart->setAnimationOptions(QChart::AllAnimations);


    /*
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 10);
    chart->setAxisY(axisY);
    */

    /*
        // Change the x axis categories
        QCategoryAxis *axisX = new QCategoryAxis();
        axisX->append("1986",0);
        axisX->append("1987",1);
        axisX->append("1988",2);
        axisX->append("1989",3);
        axisX->append("1990",4);
        axisX->append("1991",5);
        axisX->append("1992",6);
        chart->setAxisX(axisX, series);
        */

    // Used to display the chart

    this->setChart(chart);
    this->chartCounter = 7;
    return(chart);
}




void RecordWidgetGraph::bitrateInfoUpdate (FinalStatus status){

    double tempInt = status.getBitrate();
    //Appends Mbit/s
    lineSeries->append(chartCounter, tempInt/ 1000000);
    this->chart()->removeSeries(lineSeries);
    this->chart()->addSeries(lineSeries);
    this->repaint();
    chartCounter++;
}



