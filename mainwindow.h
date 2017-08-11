#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "Status/finalstatus.h"
#include "Configuration/workerconfiguration.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static QList<Interface> interfaces;


public slots:
    void closeEvent(QCloseEvent *event);
    void loadSettings();
    void saveSettings();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
