#ifndef PRODUCT_H
#define PRODUCT_H

#include <QByteArray>

extern "C" {
    #include <pcap.h>
}

class Product
{
protected:


public:
    enum ProductType { NORMAL, END, STOP, LOOP };
    QByteArray data;
    QByteArray header;
    long double packetRate; // for ts packets
    int size;
    ProductType type = NORMAL;
    Product() : size(0), type(STOP) {}
    Product(ProductType type) : type(type) {}
    Product(QByteArray data) : data(data) {}
    Product(const char *data, int size) : data(QByteArray(data, size)) {}
    Product(const u_char *data, const char *header, int size) :
        data((const char*)data, size), header(header, sizeof(pcap_pkthdr)), size(size) {}
    ~Product() {}

    QByteArray getData() { return data; }
};

#endif // PRODUCT_H
