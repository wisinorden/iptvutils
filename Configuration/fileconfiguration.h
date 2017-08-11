#ifndef FILECONFIGURATION_H
#define FILECONFIGURATION_H

#include <QString>

class FileConfiguration
{
public:
    enum FileFormat {PCAP, TS, INVALID};
protected:
    const QString filename;
    const FileFormat fileType;
public:
    FileConfiguration() : filename(""), fileType(INVALID) {}
    FileConfiguration(QString filename, FileFormat fileType) : filename(filename), fileType(fileType) {}

    QString getFilename() const { return filename; }
    FileFormat getFiletype() const { return fileType; }
};

#endif // FILECONFIGURATION_H
