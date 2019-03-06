#ifndef PCAPFILTER_H
#define PCAPFILTER_H

#include <QString>

class PcapFilter
{
public:
    PcapFilter();
    static QString generateFilter(QString host, unsigned short port, bool rtpFec);
    static QString generateFilter(QString hostport, bool rtpFec);
};

#endif // PCAPFILTER_H
