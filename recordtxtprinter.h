#ifndef RECORDTXTPRINTER_H
#define RECORDTXTPRINTER_H
#include<QObject>

#include <QFile>
#include <QTextStream>


class RecordTxtPrinter
{
public:
    RecordTxtPrinter();
    void printToFile(QString text, bool firstRound, QString currentFilename);

protected:
};

#endif // RECORDTXTPRINTER_H
