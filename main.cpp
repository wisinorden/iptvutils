#include "mainwindow.h"
#include <QApplication>
#include <QtCore>
#include "Status/status.h"
#include "Status/finalstatus.h"
#include "Status/analyzerstatus.h"
#include "Status/workerstatus.h"

#include <QMutex>

#ifndef QT_DEBUG
QTextStream *out = 0;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    static QMutex mutex;
    QMutexLocker locker(&mutex);

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        *out << "[Debug] " << localMsg << "\n";
        break;
    case QtInfoMsg:
        *out << "[Info] " << localMsg << "\n";
        break;
    case QtWarningMsg:
        *out << "[Warning] " << localMsg << "\n";
        break;
    case QtCriticalMsg:
        *out << "[Critical] " << localMsg << "\n";
        break;
    case QtFatalMsg:
        *out << "[Fatal] " << localMsg << "\n";
        out->flush();
        abort();
        break;
    }
    out->flush();
}
#endif // QT_DEBUG

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName("WISI Norden AB");
    QCoreApplication::setOrganizationDomain("wisi.se");
    QCoreApplication::setApplicationName("IPTV Utilities");

    qRegisterMetaType<Status>();
    qRegisterMetaType<WorkerStatus>();
    qRegisterMetaType<FinalStatus>();
    qRegisterMetaType<AnalyzerStatus>();

    QApplication a(argc, argv);

    #ifndef QT_DEBUG
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (path.length() > 0) {
        QString fileName = path + "/IPTVUtils.log";
        QDir dir;
        dir.mkpath(path);
        QFile *log = new QFile(fileName);
        if (log->open(QIODevice::WriteOnly | QIODevice::Text)) {
            out = new QTextStream(log);
            qInstallMessageHandler(myMessageOutput);
        } else {
            qDebug() << "Error opening log file '" << fileName << "'. All debug output redirected to console.";
        }
    }
    #endif // QT_DEBUG

    MainWindow w;
    w.show();
    qInfo() << QThread::currentThread();


    return a.exec();
}
