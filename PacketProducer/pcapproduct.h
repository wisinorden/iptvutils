#ifndef PCAPPRODUCT_H
#define PCAPPRODUCT_H

#include <QByteArray>

extern "C" {
    #include <pcap.h>
}

class PcapProduct
{
public:
    enum ProductType { NORMAL, END, STOP, LOOP };
    QByteArray data;
    QByteArray header;
    int size;
    ProductType type = NORMAL;

    PcapProduct() : size(0), type(STOP) {}
    PcapProduct(ProductType type) : type(type) {}
    ~PcapProduct() {
        //qInfo("PcapProduct descructor ~~");
    }
    PcapProduct(const u_char *data, const char *header, int size) :
        data((const char*)data, size), header(header, sizeof(pcap_pkthdr)), size(size) {
        //qInfo("Constructor PcapProduct. %i", sizeof(pcap_pkthdr));
    }

//    PcapProduct(const PcapProduct &obj) { //: data(obj.data), header(obj.header) {
//        //data = obj.data;
//        //header = obj.header;
//        data.setRawData(obj.data.data(), obj.size);
//        header.setRawData(obj.header.data(), sizeof(pcap_pkthdr));
//        size = obj.size;
//        //qInfo("Copy constructor PcapProduct.");
//     }
//    PcapProduct& operator=( const PcapProduct& rhs )
//     {
//        //qInfo("Assignment of PcapProduct.");
//        //data = rhs.data;
//        //header = rhs.header;
//        data.setRawData(rhs.data.data(), rhs.size);
//        header.setRawData(rhs.header.data(), sizeof(pcap_pkthdr));
//        size = rhs.size;
//        return *this;
//     }

//    QByteArray getData() { return data; }
//    QByteArray getHeader() { return header; }
};

#endif // PCAPPRODUCT_H
