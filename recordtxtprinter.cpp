#include "recordtxtprinter.h"
#include <QFile>
#include <QTextStream>

RecordTxtPrinter::RecordTxtPrinter()
{

}



void RecordTxtPrinter::printToFile(QFile *file, QString text, QString currentFileName, QString streamIpAdress, quint8 currentIterationIndex){


      if(!containedIndexes.contains(currentIterationIndex)){

        QString filename = currentFileName;

         streamIpAdress.replace('.', '_');
         filename.remove(".pcap");
         filename.remove(".ts");
         filename.append("_" +streamIpAdress);
         filename.append(".csv");

        file->setFileName(filename);

        if ( file->open(QIODevice::Truncate | QIODevice::Text | QIODevice::WriteOnly) )
        {
            QTextStream stream( file );

            QString s1 = "Timestamp,";
            QString s2 = "Bitrate," ;
            QString s3 = "IAT deviation";

            QString string = ( s1+ s2 + s3 );
            stream <<  string << endl;

            containedIndexes.append(currentIterationIndex);
        }



    }

//    if ( file.open(QIODevice::Append) )
    {
        QTextStream stream( file );
        stream << text << endl;
    }
}
