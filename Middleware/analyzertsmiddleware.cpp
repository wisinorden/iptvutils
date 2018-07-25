#include "analyzertsmiddleware.h"

unsigned const MP2T_SIZE = 188;
unsigned const MP2T_PCR_CLOCK = 27000000;

void AnalyzerTsMiddleware::init() {
    qInfo("AnalyzerTsMiddleware init");
    this->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::started, this,
            &AnalyzerTsMiddleware::run);
    connect(this, &AnalyzerTsMiddleware::finished,
            &runnerThread, &QThread::quit);
}

void AnalyzerTsMiddleware::start() {
    qInfo("AnalyzerTsMiddleware starting");
    runnerThread.start();
}

void AnalyzerTsMiddleware::stop() {
    stopping = true;
    buffer.stop();
}

void AnalyzerTsMiddleware::run() {
    qInfo("starting AnalyzerTsMiddleware");
    emit started();
    bufferProducts();
    qInfo("AnalyzerTsMiddleware finished");
    emit finished();
    this->thread()->quit();
}

void AnalyzerTsMiddleware::bufferProducts() {

}

Product AnalyzerTsMiddleware::getProduct() {
    return buffer.pop();
}
