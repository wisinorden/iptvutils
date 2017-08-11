#ifndef FILEOUTPUTCONFIGURATION_H
#define FILEOUTPUTCONFIGURATION_H

#include <QString>
#include "fileconfiguration.h"

class FileOutputConfiguration : public FileConfiguration
{
public:
    FileOutputConfiguration() {}
    FileOutputConfiguration(QString filename, FileFormat fileType) : FileConfiguration(filename, fileType) {}
};

#endif // FILEOUTPUTCONFIGURATION_H
