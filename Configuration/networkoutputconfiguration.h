#ifndef NETWORKOUTPUTCONFIGURATION_H
#define NETWORKOUTPUTCONFIGURATION_H

#include "networkconfiguration.h"

class NetworkOutputConfiguration : public NetworkConfiguration
{
public:
    enum RewriteFlags {
        REWRITE_NONE     = 0,
        REWRITE_DST_HOST = 1,
        REWRITE_DST_PORT = 2,
        REWRITE_DST      = 3,
        REWRITE_SRC_HOST = 4,
        REWRITE_SRC_PORT = 8,
        REWRITE_SRC    = 0xC,
        REWRITE_ALL    = 0xF
    }RewriteFlags;
private:
    const int rewriteHeaders;
public:
    NetworkOutputConfiguration() : rewriteHeaders(false) {}
    NetworkOutputConfiguration(
            Interface device,
            int rewriteHeaders = 0,
            QString host = QString(),
            quint16 port = 1234) :
        NetworkConfiguration(device, host, port),
        rewriteHeaders(rewriteHeaders)  {}

    bool rewriteDstHost() const { return (rewriteHeaders & 0x1) == 0x1; }
    bool rewriteDstPort() const { return (rewriteHeaders & 0x2) == 0x2; }
    bool rewriteSrcHost() const { return (rewriteHeaders & 0x4) == 0x4; }
    bool rewriteSrcPort() const { return (rewriteHeaders & 0x8) == 0x8; }
};

#endif // NETWORKOUTPUTCONFIGURATION_H
