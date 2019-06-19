#include "recordtxtprinter.h"
#include <QFile>
#include <QTextStream>

RecordTxtPrinter::RecordTxtPrinter()
{

}



void RecordTxtPrinter::printToFile(QString text){
    QString filename="/home/marko.marinkovic/Documents/Data.txt";
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream << text << endl;
    }
}
