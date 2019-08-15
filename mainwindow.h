#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QApplication>
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

protected :
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);



public slots:
    void closeEvent(QCloseEvent *event);
    void loadSettings();
    void saveSettings();


private slots:
    void on_actionExit_triggered();


    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
