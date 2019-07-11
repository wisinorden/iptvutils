#include "recordtxtprinter.h"
#include <QFile>
#include <QTextStream>

RecordTxtPrinter::RecordTxtPrinter()
{


}



void RecordTxtPrinter::printToFile( QString text, bool firstRound, QString currentFileName, QString streamIpAdress){

   QString filename = currentFileName;

    streamIpAdress.replace('.', '_');
    filename.remove(".pcap");
    filename.append("_" +streamIpAdress);
    filename.append(".csv");

    QFile file(filename);


    if(firstRound){
        file.resize(0);

        if ( file.open(QIODevice::Append) )
        {
            QTextStream stream( &file );

            QString s1 = "Timestamp,";
            QString s2 = "Bitrate," ;
            QString s3 = "IAT deviation";

            QString string = ( s1+ s2 + s3 );
            stream <<  string << endl;
        }

    }

    if ( file.open(QIODevice::Append) )
    {
        QTextStream stream( &file );
        stream << text << endl;
    }
}
