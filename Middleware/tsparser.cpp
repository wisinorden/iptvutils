#include "tsparser.h"

void TsParser::parse(quint8 *tsDataIn) {
    tsData = tsDataIn;
    hdr.sync = tsData[0];
    hdr.TEI = tsData[1] >> 7;
    hdr.PSUI = tsData[1] >> 6;
    hdr.prio = tsData[1] >> 5;
    hdr.PID = ((tsData[1]&0x1f) << 8) + (tsData[2]&0xFF);
    hdr.TSC = (tsData[3] & 0xC0) >> 6;
    hdr.AFC = (tsData[3] & 0x30) >> 4;
    hdr.CC = tsData[3] & 0x0f;

    quint8 *payload = tsData+4;

    if (hdr.AFC >= 2) {
        af.adaptation_field_length = *payload;
        payload += af.adaptation_field_length+1;

        if (af.adaptation_field_length > 0) {
            af.discontinuity_indicator = (tsData[5] & 0x80) >> 7;
            af.random_access_indicator = (tsData[5] & 0x40) >> 6;
            af.elementary_stream_priority_indicator = (tsData[5] & 0x20) >> 5;
            af.PCR_flag = (tsData[5] & 0x10) >> 4;
            af.OPCR_flag = (tsData[5] & 0x08) >> 3;
            af.splicing_point_flag = (tsData[5] & 0x04) >> 2;
            af.transport_private_data_flag = (tsData[5] & 0x02) >> 1;
            af.adaptation_field_extension_flag = tsData[5] & 0x01;
        }
    }

    // if PAT
    if (hdr.PID == 0) {
        quint8 psi_offset = payload[0];

        pa_section.table_id = payload[psi_offset+1];
        pa_section.section_syntax_indicator = payload[psi_offset+2] >> 7;
        pa_section.section_length = ((payload[psi_offset+2] & 0x0F) << 8) + payload[psi_offset+3];
        pa_section.transport_stream_id = (payload[psi_offset+4] << 8) + payload[5];
        pa_section.version_number = (payload[psi_offset+6] & 0x3E) >> 1;
        pa_section.current_next_indicator = payload[psi_offset+6] & 0x01;
        pa_section.section_number = payload[psi_offset+7];
        pa_section.last_section_number = payload[psi_offset+8];

//        qInfo("length: %i, version 0x%x, TSID 0x%x, programs: %i",
//                pa_section.section_length,
//                pa_section.version_number,
//                pa_section.transport_stream_id,
//                (pa_section.section_length-9)/4
//                );

        for (int i = 0; i < (pa_section.section_length-9)/4; i++) {
            //qInfo("prognum: %x %x", tsData[pas_pointer+8+i*4], tsData[pas_pointer+9+i*4]);
            quint16 progNum = (payload[psi_offset+9+i*4] << 8) + payload[psi_offset+10+i*4];
            quint16 progPid = ((payload[psi_offset+11+i*4] & 0x1F) << 8) + payload[psi_offset+12+i*4];
//            qInfo("Found program: id: 0x%x(%u), PID: 0x%x(%u)",
//                  progNum, progNum,
//                  progPid, progPid);
            programPidMap[progPid].programNumber = progNum;
            programPidMap[progPid].programPid = progPid;
        }
    }
    else if (programPidMap.contains(hdr.PID)) {
        // is a PMT
        quint8 psi_offset = payload[0];

        pm_section.table_id = payload[psi_offset+1];
        pm_section.section_syntax_indicator = (payload[psi_offset+1] & 0x80) >> 7;
        pm_section.section_length = ((payload[psi_offset+2] & 0x0F) << 8) + payload[psi_offset+3];
        pm_section.program_number = (payload[psi_offset+4] << 8) + payload[psi_offset+5];
        pm_section.version_number = (payload[psi_offset+6] & 0x3E) >> 1;
        pm_section.current_next_indicator = payload[psi_offset+6] & 0x01;
        pm_section.section_number = payload[psi_offset+7];
        pm_section.last_section_number = payload[psi_offset+8];
        pm_section.PCR_PID = ((payload[psi_offset+9] & 0x1F) << 8) + payload[psi_offset+10];
        pm_section.program_info_length = ((payload[psi_offset+11] & 0x0F) << 8) + payload[psi_offset+12];

        programPidMap[hdr.PID].PCR_PID = pm_section.PCR_PID;
        //qInfo("Found pcr_pid for program %i, pid: 0x%x", pm_section.program_number, pm_section.PCR_PID);
    }
}

//sync: 8,    // 0x47 sync byte
//TEI : 1,    // Transport Error Indicator
//PSUI: 1,    // Payload Unit Start Indicator
//prio: 1,    // Transport Priority
//PID : 13,   // Packet Id
//TSC : 2,    // Transport Scrambling Control
//AFC : 2,    // Apatation Field Control
//CC  : 4;    // Continuity Counter

// table_id: 8;
// section_syntax_indicator : 1,
// section_length: 12;
// transport_stream_id: 16;
// version_number: 5,
// current_next_indicator: 1;
// section_number: 8;
// last_section_number: 8;

void TsParser::addCCOffset(quint8 offset) {
    hdr.CC += offset;
    hdr.CC &= 0x0f;

    // Keep all that is not CC, add new offsetted CC
    tsData[3] = (tsData[3] & 0xF0) + hdr.CC;
}

void TsParser::setDiscontiniousFlag() {
    quint8 adaptation_field_length = tsData[4];
    if (adaptation_field_length > 0) {
        tsData[5] = tsData[5] | 0x80; // sets discontinious bit
    }
}
