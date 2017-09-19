#ifndef CONVERTWIDGET_H
#define CONVERTWIDGET_H

#include "Recorder/pcaptsconverter.h"
#include "Status/workerstatus.h"
#include <QWidget>

namespace Ui {
class ConvertWidget;
}

class ConvertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConvertWidget(QWidget *parent = 0);
    ~ConvertWidget();

    void loadSettings();
    void saveSettings();
    void startConvert(WorkerConfiguration::WorkerMode mode = WorkerConfiguration::NORMAL_MODE);

    static PcapTsConverter *pcapTsConverter;

private:
    Ui::ConvertWidget *ui;
    QHash<quint64, StreamInfo> streams;
    StreamId selectedStreamId;

private slots:
    void convertFinished();

    void on_convertExpandPCAPFilterButton_toggled(bool);
    void on_convertStartBtn_clicked();
    void on_convertFromFileDialog_clicked();
    void on_convertToFileDialog_clicked();
    void on_treeWidget_itemSelectionChanged();

public slots:
    void convertStatusChanged(FinalStatus status);
    void convertWorkerStatusChanged(WorkerStatus status);
};

#endif // CONVERTWIDGET_H
