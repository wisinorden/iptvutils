#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>

class QGestureEvent;

QT_CHARTS_USE_NAMESPACE

class Chart : public QChart
{
public:
    explicit Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    ~Chart();

protected:
    bool sceneEvent(QEvent *event);

private:
    bool gestureEvent(QGestureEvent *event);

private:

};

#endif // CHART_H
