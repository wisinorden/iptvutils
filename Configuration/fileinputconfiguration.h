#ifndef FILEINPUTCONFIGURATION_H
#define FILEINPUTCONFIGURATION_H

#include <QString>
#include "fileconfiguration.h"

class FileInputConfiguration : public FileConfiguration
{
public:
    enum LoopType {
        LOOP_NONE,
        LOOP_SIMPLE,
        LOOP_DISCONTINUITY_FLAG,
        LOOP_PCR_REWRITE,
    };
private:
    const QString filter;
    LoopType loop;

public:
    FileInputConfiguration() {}
    FileInputConfiguration(
            QString filename,
            FileFormat fileType,
            QString filter = QString(""),
            LoopType loop = LOOP_NONE) :
        FileConfiguration(filename, fileType), filter(filter), loop(loop) {}

    QString getFilter() const { return filter; }
    LoopType getLoopStyle() { return loop; }
};

#endif // FILEINPUTCONFIGURATION_H
