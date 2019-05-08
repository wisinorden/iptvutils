#ifndef GRAPHCHART_H
#define GRAPHCHART_H


#include <QtCharts/QChart>
#include <QtCore/QTimer>
//#include <QtCore/QRandomGenerator>


QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class GraphChart: public QChart
{
    Q_OBJECT
public:
    GraphChart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~GraphChart();

public slots:
    void handleTimeout();

private:
    QTimer m_timer;
    QSplineSeries *m_series;
    QStringList m_titles;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    qreal m_step;
    qreal m_x;
    qreal m_y;
};

#endif /* GRAPHCHART_H */
