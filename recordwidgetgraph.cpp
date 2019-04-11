#include "recordwidgetgraph.h"

// This class deals with the graph in the recordwidget

RecordWidgetGraph::RecordWidgetGraph()
{

}


QChart* RecordWidgetGraph::setupGraph(){

        QLineSeries *series = new QLineSeries();
        series->append(0, 16);
        series->append(1, 25);
        series->append(2, 24);
        series->append(3, 19);
        series->append(4, 33);
        series->append(5, 25);
        series->append(6, 34);

        // Create chart, add data, hide legend, and add axis
        QChart * chart = new QChart();
        chart->legend()->hide();
        chart->addSeries(series);
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
        series->setPen(pen);

        chart->setAnimationOptions(QChart::AllAnimations);

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


        return(chart);
    }
