#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QDebug>

extern "C" {
    #include <pcap.h>
}

/* 4 bytes IP address */
typedef struct ip_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* Ethernet header */
typedef struct eth_header{
    u_short dst_mac1;
    u_short dst_mac2;
    u_short dst_mac3;
    u_short src_mac1;
    u_short src_mac2;
    u_short src_mac3;
    u_short eth_type;
}eth_header;

/* IPv4 header */
typedef struct ip_header{
    u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char  tos;            // Type of service
    u_short tlen;           // Total length
    u_short identification; // Identification
    u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char  ttl;            // Time to live
    u_char  proto;          // Protocol
    u_short crc;            // Header checksum
    ip_address  saddr;      // Source address
    ip_address  daddr;      // Destination address
    u_int   op_pad;         // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header{
    u_short sport;          // Source port
    u_short dport;          // Destination port
    u_short len;            // Datagram length
    u_short crc;            // Checksum
}udp_header;

/* RTP header*/
typedef struct rtp_header{
    u_char v_p_x_cc;        // version, padding, extension, csrc count (4 bits)
    u_char m_pt;            // marker bit, payload type (7 bits)
    u_short seq;            // sequence number
    u_int ts;               // timestamp
    u_int ssrc;             // synchronization source id
    u_int csrc[1];          // CSRC list
}rtp_header;

#define RTP_V(x) ((x & 0xC0) >> 6)
#define RTP_X(x) ((x & 0x10) >> 4)
#define RTP_CC(x) (x & 0x0F)

/* RTP extension*/
typedef struct rtp_extension{
    u_short etype;          // extension type
    u_short elen;           // extension length
    u_char epayload[1];     // extension payload
}rtp_extension;

/* RTP packet*/
typedef struct rtp_packet{
    rtp_header *header;
    rtp_extension *extension;
}rtp_packet;

class PacketParser
{
public:
    PacketParser() {}
    PacketParser(const pcap_pkthdr *header, const u_char *pkt_data) {
        this->parse(header, pkt_data);
    }
    void parse(const pcap_pkthdr *header, const u_char *pkt_data);
    u_short calc_ip_checksum();
    void unsetUdpChecksum();
    void changeSourceIpAndMac(QString host, quint16 port, quint64 mac);
    void changeMulticastDestination(QString host);
    void changePort(quint16 port);

    //struct tm ltime;
    //char timestr[16];
    eth_header *eh;
    ip_header *ih;
    udp_header *uh;
    rtp_packet rp;
    u_int eh_len;
    u_int ip_len;
    u_int rp_len;
    u_short sport,dport;
    //time_t local_tv_sec;
    u_char *rp_data;
    u_int rp_data_len;
    u_char *data;
    u_int data_len;
};

#endif // PACKETPARSER_H
