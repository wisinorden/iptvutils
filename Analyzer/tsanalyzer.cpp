#include "tsanalyzer.h"

#ifdef QT_DEBUG
    //#define ANALYZER_CONSOLE_OUTPUT
#endif

bool TsAnalyzer::validSyncByte() {
    if (tsParser.hdr.sync != 0x47) {
        tsErrors->errorCounter[TsErrors::Sync_byte_error]++;
#ifdef ANALYZER_CONSOLE_OUTPUT
        qInfo("Sync byte error");
#endif // ANALYZER_CONSOLE_OUTPUT
        return false;
    }
    return true;
}

bool TsAnalyzer::ccErrorDetect() {
    if (tsParser.hdr.PID == 0x1FFF) {
        // Null packets cannot have CC-Drops
    }
    else if (!pidMap->contains(tsParser.hdr.PID)) {
        // Have not seed pid before, cannot have dropped
    }
    else if (pidMap->value(tsParser.hdr.PID).cc == tsParser.hdr.CC && tsParser.hdr.AFC == 0x2) {
        // Same assumption as in Wireshark mp2t
        // "Its allowed that (cc_prev == cc_curr) if adaptation field"
    }
    else if (((pidMap->value(tsParser.hdr.PID).cc+1)&0x0F) != tsParser.hdr.CC) {
        (*pidMap)[tsParser.hdr.PID].ccErrors++;
        tsErrors->errorCounter[TsErrors::Continuity_count_error]++;
#ifdef ANALYZER_CONSOLE_OUTPUT
        qInfo("%lli: CC error: pid: 0x%x (%i), count %lli, CC: %i, : last CC: %i",
              packetNumber,
              tsParser.hdr.PID,
              tsParser.hdr.PID,
              pidMap->value(tsParser.hdr.PID).ccErrors,
              tsParser.hdr.CC,
              pidMap->value(tsParser.hdr.PID).cc);
#endif // ANALYZER_CONSOLE_OUTPUT
        return true;
    }
    return false;
}
