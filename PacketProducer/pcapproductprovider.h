#ifndef PCAPPRODUCTPROVIDER_H
#define PCAPPRODUCTPROVIDER_H

#include "pcapproduct.h"

class PcapProductProvider {
protected:
    PcapProductProvider *prevProvider = NULL;
    PcapProductProvider *nextProvider = NULL;

public:
    PcapProductProvider* addNext(PcapProductProvider* next) {
        this->nextProvider = next;
        next->prevProvider = this;
        return next;
    }

    PcapProductProvider* addToEnd(PcapProductProvider* next) {
        // this provider is not last in chain, add to nextProvider
        if (this->nextProvider != NULL) {
            return this->nextProvider->addToEnd(next);
        }
        return addNext(next);
    }

    virtual PcapProduct getProduct() {
        return prevProvider->getProduct();
    }
};

#endif // PCAPPRODUCTPROVIDER_H
