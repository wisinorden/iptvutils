#ifndef WORKERCONFIGURATION_H
#define WORKERCONFIGURATION_H

#include <QString>
#include "networkinputconfiguration.h"
#include "networkoutputconfiguration.h"
#include "fileinputconfiguration.h"
#include "fileoutputconfiguration.h"

class WorkerConfiguration
{
public:
    enum WorkerMode {
        NORMAL_MODE,
        ANALYSIS_MODE_OFFLINE,
        ANALYSIS_MODE_LIVE
    };
    enum WorkerType {
        NETWORK, FILE
    };
private:
    const WorkerType inputType;
    const NetworkInputConfiguration networkInput;
    const FileInputConfiguration fileInput;

    const WorkerType outputType;
    const NetworkOutputConfiguration networkOutput;
    const FileOutputConfiguration fileOutput;

    const WorkerMode workerMode;

public:
    // Network -> Network configuration
    WorkerConfiguration(NetworkInputConfiguration input, NetworkOutputConfiguration output, WorkerMode mode = NORMAL_MODE) :
        inputType(NETWORK), networkInput(input), outputType(NETWORK), networkOutput(output), workerMode(mode) {}

    // Network -> File configuration
    WorkerConfiguration(NetworkInputConfiguration input, FileOutputConfiguration output, WorkerMode mode = NORMAL_MODE) :
        inputType(NETWORK), networkInput(input), outputType(FILE), fileOutput(output), workerMode(mode) {}

    // File -> Network configuration
    WorkerConfiguration(FileInputConfiguration input, NetworkOutputConfiguration output, WorkerMode mode = NORMAL_MODE) :
        inputType(FILE), fileInput(input), outputType(NETWORK), networkOutput(output), workerMode(mode) {}

    // File -> File configuration
    WorkerConfiguration(FileInputConfiguration input, FileOutputConfiguration output, WorkerMode mode = NORMAL_MODE) :
        inputType(FILE), fileInput(input), outputType(FILE), fileOutput(output), workerMode(mode) {}

    WorkerType getInputType() const { return inputType; }
    WorkerType getOutputType() const { return outputType; }

    NetworkInputConfiguration getNetworkInput() const { return networkInput; }
    NetworkOutputConfiguration getNetworkOutput() const { return networkOutput; }

    FileInputConfiguration getFileInput() const { return fileInput; }
    FileOutputConfiguration getFileOutput() const { return fileOutput; }

    WorkerMode getWorkerMode() const { return workerMode; }

};

#endif // WORKERCONFIGURATION_H
