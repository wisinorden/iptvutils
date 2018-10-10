#ifndef PCAPPRODUCTPROVIDER_H
#define PCAPPRODUCTPROVIDER_H

#include "pcapproduct.h"
#include "tsproduct.h"

class ProductProvider {
protected:
    ProductProvider *prevProvider = NULL;
    ProductProvider *nextProvider = NULL;

public:
    ProductProvider* addNext(ProductProvider* next) {
        this->nextProvider = next;
        next->prevProvider = this;
        return next;
    }

    ProductProvider* addToEnd(ProductProvider* next) {
        // this provider is not last in chain, add to nextProvider
        if (this->nextProvider != NULL) {
            return this->nextProvider->addToEnd(next);
        }
        return addNext(next);
    }

    virtual Product getProduct() {
        return prevProvider->getProduct();
    }
};

#endif // PCAPPRODUCTPROVIDER_H
