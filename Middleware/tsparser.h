#ifndef TSPARSER_H
#define TSPARSER_H

#include "programinfo.h"
#include <QtNumeric>
#include <QMap>

typedef struct TsHeader {
    quint32 sync: 8,    // 0x47 sync byte
            TEI : 1,    // Transport Error Indicator
            PSUI: 1,    // Payload Unit Start Indicator
            prio: 1,    // Transport Priority
            PID : 13,   // Packet Id
            TSC : 2,    // Transport Scrambling Control
            AFC : 2,    // Apatation Field Control
            CC  : 4;    // Continuity Counter

}TsHeader;

typedef struct TsAdaptField {
    quint8 adaptation_field_length ;
    quint8 discontinuity_indicator : 1,
           random_access_indicator : 1,
           elementary_stream_priority_indicator : 1,
           PCR_flag : 1,
           OPCR_flag: 1,
           splicing_point_flag  : 1,
           transport_private_data_flag : 1,
           adaptation_field_extension_flag : 1;
}TsAdaptField;

/*
typedef struct TsAdaptExtField {
    quint8 len;         // Bytes of header
    quint8 LTW : 1,     // Legal time Window flag
           PRF : 1,     // Piecewise rate flag
           SSF : 1,     // Seamless splice flag
               : 5;     // reserved
}TsAdaptExtField;*/

typedef struct PSI_Pointer {
    quint8 pointer_field; // nr of bytes until first section
}PSI_Pointer;

typedef struct PA_Data {
    quint16 program_number : 16;
    quint16 program_map_PID: 13;
}PA_Data;

typedef struct PA_Section {
    quint8  table_id: 8;
    quint16 section_syntax_indicator : 1,
            section_length: 12;
    quint16 transport_stream_id: 16;
    quint8  version_number: 5,
            current_next_indicator: 1;
    quint8  section_number: 8;
    quint8  last_section_number: 8;
}PA_Section;

typedef struct PM_Section {
    quint8  table_id: 8;
    quint16 section_syntax_indicator : 1,
            section_length: 12;
    quint16 program_number: 16;
    quint8  version_number: 5,
            current_next_indicator: 1;
    quint8  section_number: 8;
    quint8  last_section_number: 8;
    quint16 PCR_PID: 13;
    quint16 program_info_length: 12;
}PM_Section;


class TsParser
{
private:
    quint8* tsData;

public:
    TsParser() {}
    void parse(quint8* tsDataIn);
    void addCCOffset(quint8 offset);
    void setDiscontiniousFlag();

    TsHeader hdr;
    TsAdaptField af;
    PA_Section pa_section;
    PM_Section pm_section;

    // program pid as key
    QMap<quint16, ProgramInfo> programPidMap;
};

#endif // TSPARSER_H
