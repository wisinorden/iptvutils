#include "packetparser.h"
#include <QtNetwork>

#ifdef __linux__
#include <arpa/inet.h>
#endif

void PacketParser::parse(const pcap_pkthdr *header, const u_char *pkt_data)
{
    //local_tv_sec = header->ts.tv_sec;
    //localtime_s(&ltime, &local_tv_sec);
    //strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);

    /* retrieve the position of the eth header */
    eh = (eth_header *)pkt_data;

    switch(ntohs(eh->eth_type)) {
        case 0x0800: // IPv4 without VLAN
            eh_len = 14;
            break;
        case 0x8100: // 802.1Q tagged
            eh_len = 18; // adds 4 bytes
            break;
        case 0x88a8: // 802.1ad
            if (ntohs(*(pkt_data+4)) == 0x8100) { // double tagged with 802.1Q
                eh_len = 22; // adds 4 + 4 bytes
            }
            else {
                eh_len = 18; // adds 4 bytes
            }
            break;
        default:
            eh_len = 14; // assume 14 if else not specified (IPv4)
    }

    /* retrieve the position of the ip header */
    ih = (ip_header *) (pkt_data + eh_len);

    /* retrieve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;
    uh = (udp_header *) ((u_char*)ih + ip_len);

    /* retrieve the position of a potential rtp header */
    rp_len = 0;
    rp.header = (rtp_header*)((u_char*)uh + 8); // UDP header always 8 bytes
    if (RTP_V(rp.header->v_p_x_cc) == 2) { // RTP v2
        //qInfo("appears to be rtp, vpxcc: 0x%02x, cc: %i, v%i", rp.header->v_p_x_cc, RTP_CC(rp.header->v_p_x_cc), RTP_V(rp.header->v_p_x_cc));
        rp_len = 12 + RTP_CC(rp.header->v_p_x_cc) * 4; // 12 bytes minimum, cc * number of bytes per CSRC (4)
        if (RTP_X(rp.header->v_p_x_cc)) {
            rp.extension = (rtp_extension*)(rp.header+rp_len);
            rp_len += rp.extension->elen;
        }
    }

    // check if message without rtp is correct size for mpeg 2 ts packets
    // else assume it has rtp
    // 8 is the size of udp header
    if ((ntohs(uh->len)-8) % 188 == 0) {
        rp_len = 0;
    }
    //qInfo("uh-len: %i, rp_len %i", ntohs(uh->len), rp_len);

    rp_data = (u_char*)rp.header;
    u_int rp_head_len = (u_int)(rp_data - pkt_data);
    rp_data_len = (u_int)header->len - rp_head_len + rp_len;

    data = (u_char*)((u_char*)rp.header + rp_len);
    u_int head_len = (u_int)(data - pkt_data);
    data_len = (u_int)header->len - head_len;

    /* convert from network byte order to host byte order */
    sport = ntohs( uh->sport );
    dport = ntohs( uh->dport );

    //qInfo("rtp_header len %i", sizeof(rtp_header));
}

u_short PacketParser::calc_ip_checksum() {
    u_int sum = 0;  /* assume 32 bit, 16 bit short */
    u_int len = (ih->ver_ihl & 0x0F) * 4;

    u_short* ip = (u_short*)ih;

    while(len > 1){
        sum += *(ip)++;
        if(sum & 0x80000000)   /* if high order bit set, fold */
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    sum -= ih->crc;

    if(len)       /* take care of left over byte */
        sum += (u_short) *ip;

    while(sum>>16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    ih->crc = ~sum;
    return ~sum;
}

void PacketParser::unsetUdpChecksum() {
    this->uh->crc = 0;
}

void PacketParser::changeSourceIpAndMac(QString host, quint16 port, quint64 mac) {
    QHostAddress hosta = QHostAddress(host);
    u_int ipv4addr = hosta.toIPv4Address();

    ih->saddr.byte1 = (ipv4addr & 0xFF000000) >> 24;
    ih->saddr.byte2 = (ipv4addr & 0x00FF0000) >> 16;
    ih->saddr.byte3 = (ipv4addr & 0x0000FF00) >> 8;
    ih->saddr.byte4 = (ipv4addr & 0x000000FF) >> 0;

    eh->src_mac1 = htons((mac & 0xFFFF00000000) >> 32);
    eh->src_mac2 = htons((mac & 0x0000FFFF0000) >> 16);
    eh->src_mac3 = htons((mac & 0x00000000FFFF) >> 0);

    uh->sport = htons(port);

    // recalculate the ip header checksum
    calc_ip_checksum();
    // unset rather than calculate udp checksum
    unsetUdpChecksum();
}

void PacketParser::changeMulticastDestination(QString host) {
    QHostAddress hosta = QHostAddress(host);
    u_int ipv4addr = hosta.toIPv4Address();
    u_short mac2 = (u_short)((ipv4addr & 0xFFFF0000) >> 16);
    u_short mac3 = (u_short)(ipv4addr & 0x0000FFFF);

    // Change destination ip-address
    //parser.ih->daddr = (ip_address)ipv4addr;
    ih->daddr.byte1 = (ipv4addr & 0xFF000000) >> 24;
    ih->daddr.byte2 = (ipv4addr & 0x00FF0000) >> 16;
    ih->daddr.byte3 = (ipv4addr & 0x0000FF00) >> 8;
    ih->daddr.byte4 = (ipv4addr & 0x000000FF) >> 0;

    // Destination mac-address for multicast holds 23 lowest bits from ip-address
    // Data in is in network byte order, need to account for that
    eh->dst_mac2 = (eh->dst_mac2 & htons(0xFF80)) | htons(mac2 & 0x007F);
    eh->dst_mac3 = htons(mac3);

    // recalculate the ip header checksum
    calc_ip_checksum();
}

void PacketParser::changePort(quint16 port) {
    // Change destination port
    uh->dport = htons(port);

    // unset rather than calculate udp checksum
    unsetUdpChecksum();
}
