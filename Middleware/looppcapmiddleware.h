#ifndef LOOPPCAPMIDDLEWARE_H
#define LOOPPCAPMIDDLEWARE_H

#include "pcapmiddleware.h"
#include "concurrentqueue.h"

class LoopPcapMiddleware : public PcapMiddleware
{
    Q_OBJECT
private:
    ConcurrentQueue<PcapProduct> buffer;
    void bufferProducts();
    quint64 timevalToUsec(timeval time);
    timeval usecToTimeval(quint64 time);

public:
    LoopPcapMiddleware(WorkerConfiguration config) :
            PcapMiddleware(config), buffer(MIDDLEWARE_BUFFER_SIZE) {
        init();
    }

    void init();
    PcapProduct getProduct();

protected slots:
    void run();

public slots:
    void start();
    void stop();
};

#endif // LOOPPCAPMIDDLEWARE_H
