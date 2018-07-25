#ifndef TSPRODUCT_H
#define TSPRODUCT_H

#include "product.h"

class TsProduct : public Product
{
public:
    ProductType type = NORMAL;
    TsProduct() : Product() {}
    TsProduct(ProductType type) : Product(type) {}
    TsProduct(QByteArray data) : Product(data) {}
};

#endif // TSPRODUCT_H
