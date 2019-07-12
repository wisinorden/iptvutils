#ifndef RECORDTXTPRINTER_H
#define RECORDTXTPRINTER_H
#include<QObject>

#include <QFile>
#include <QTextStream>


class RecordTxtPrinter
{
public:

   // QFile file;
    RecordTxtPrinter();
    void printToFile(QFile *file, QString text, QString currentFilename, QString streamIpAdress,quint8 currentIterationIndex);
    quint8 numberOfFiles;
    QList <quint8> containedIndexes;

protected:
};

#endif // RECORDTXTPRINTER_H
