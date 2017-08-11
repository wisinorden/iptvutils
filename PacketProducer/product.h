#ifndef PRODUCT_H
#define PRODUCT_H

#include <QByteArray>

class Product
{
protected:
    QByteArray data;

public:
    Product() {}
    Product(QByteArray data) : data(data) {}
    Product(const char *data, int size) : data(QByteArray(data, size)) {}

    QByteArray getData() { return data; }
};

#endif // PRODUCT_H
