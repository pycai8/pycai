#include <string>

#include "ITsEncoder.h"
#include "IPycaiLogger.h"

class TsHeader {
public:
    /* byte 0 */
    uint32_t sync_byte:8; // fixed value 0x47
    /* byte 1 ~ 2 */
    uint32_t transport_error_indicator:1; // 0: no bit error; 1: has bit error
    uint32_t payload_unit_start_indicator:1; // 0: no adjust byte; 1: has an adjust byte after TsHeader
    uint32_t transport_priority:1; // 0: normal priority; 1: high priority
    uint32_t pid:13; // type of ts
    /* byte 3 */
    uint32_t transport_scrambling_control:2; // 00: no encryption; other: has encryption;
    uint32_t adaptation_field_control:2; // first bit: has adaption or not; second bit: has payload or not;
    uint32_t continuity_count:4; // continuity_count = (continuity_count + 1) % 16;
    
    TsHeader()
    {
        sync_byte = 0x47;
        transport_error_indicator = 0;
        payload_unit_start_indicator = 0;
        transport_priority = 0;
        pid = 0;
        transport_scrambling_control = 0;
        adaptation_field_control = 1;
        continuity_count = 0;
    }
};

void SetPatHeader(TsHeader& hdr)
{
    hdr.playload_unit_start_indicator = 1;
    hdr.transport_priority = 1;
    hdr.pid = 0;
    hdr.adaptation_field_control = 1;
    continuity_count = 0;
}

void SetPmtHeader(TsHeader& hdr)
{
    hdr.playload_unit_start_indicator = 1;
    hdr.transport_priority = 1;
    hdr.pid = 0x81;
    hdr.adaptation_field_control = 1;
    continuity_count = 0;
}

class PatInfo {
public:
    /* byte 0 */
    uint32_t table_id:8; // fixed 0
    /* byte 1 ~ 2 */
    uint32_t section_syntax_indicator:1; // fixed 1
    uint32_t zero:1; // fixed 0
    uint32_t reserved1:2; // fixed 3
    uint32_t section_length:12; // 13 the count of usefull bytes after this , crc32 is contained.
    /* byte 3 ~ 4 */
    uint32_t transport_stream_id:16; // let it's 0
    /* byte 5 */
    uint32_t reserved2:2; // b11
    uint32_t version_number:5; // 0
    uint32_t current_next_indicator:1; // 1
    /* byte 6 */
    uint32_t section_number:8; // 0
    /* byte 7 */
    uint32_t last_section_number:8; // 0
    /* byte 8 ~ 9 */
    uint32_t program_number:16; // 1
    /* byte 10 ~ 11 */
    uint32_t reserved3:3; // b111
    uint32_t program_map_pid:13; // 0x81
    /* byte 12 ~ 15 */
    uint32_t crc32; // 0x0c8cbe32

    PatInfo()
    {
        table_id = 0;
        section_syntax_indicator = 1;
        zero = 0;
        reserved1 = 3;
        section_length = 13;
        transport_stream_id = 0;
        reserved2 = 3;
        version_number = 0;
        current_next_indicator = 1;
        section_number = 0;
        last_section_number = 0;
        program_number = 1;
        reserved3 = 7;
        program_map_pid = 0x81;
        crc32 = 0x0c8cbe32;
    }
};

class PmtInfo {
public:
    uint32_t table_id                   : 8; //固定为0x02, 表示PMT表  
    uint32_t section_syntax_indicator    : 1; //固定为0x01  
    uint32_t zero                       : 1; //0x0  
    uint32_t reserved1                 : 2; //0x03  
    uint32_t section_length : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC  
    uint32_t program_number             : 16;//0x1 指出该节目对应于可应用的Program map PID  
    uint32_t reserved2                 : 2; //0x03  
    uint32_t version_number             : 5; //0x0 指出TS流中Program map section的版本号  
    unsigned current_next_indicator  : 1; //1 当该位置1时，当前传送的Program map section可用  
     //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效  
    uint32_t section_number            : 8; //固定为0x00  
    uint32_t last_section_number      : 8; //固定为0x00  
    uint32_t reserved3               : 3; //0x07  
    uint32_t PCR_PID                   : 13; //0x810 指明TS包的PID值，该TS包含有PCR域，  
      //该PCR值对应于由节目号指定的对应节目，如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
    uint32_t reserved4            : 4;  //15 预留为0x0F  
    uint32_t program_info_length  : 12; //0 前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  
    
    uint32_t stream_type:8;
    uint32_t reserved5:3;
    uint32_t elementary_pid:13;
    uint32_t reserved6:4;
    uint32_t es_info_length:12;

    uint32_t crc32                    : 32;   

    PmtInfo()
    {
        table_id = 2;
        section_syntax_indicator = 1;
        zero = 0;
        reserved1 = 3;
        section_length = 18;
        program_number = 1;
        reserved2 = 3;
        version_number = 0;
        current_next_indicator = 1;
        section_number = 0;
        last_section_number = 0;
        reserved3 = 7;
        PCR_PID = 0x810;
        reserved4 = 15;
        program_info_length = 0;
        stream_type = 1; // h264
        reserved5 = 7;
        elementary_pid = 0x810;
        reserved6 = 15;
        es_info_length = 0;
        crc32 = 0; // ?
    }
};

class PesHeader {

};

class CTsEncoder : public ITsEncoder
{
public:
    class CFactory : public ITsEncoder::IFactory
    {
    public:
        CFactory()
        {
            RegisterFactory();
        }
        virtual ~CFactory()
        {
            UnregisterFactory();
        }

        const char* GetClassId() const override
        {
            return "CTsEncoder";
        }

        IUnknowObject* Create()
        {
            return new CTsEncoder();
        }
    };

    CTsEncoder() {}
    virtual ~CTsEncoder() {}

    const char* GetConfig(const char*) const override
    {
        return "";
    }

    bool SetConfig(const char*, const char*) override
    {
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool H264ToTs(const char* h264File, const char* tsFile) override
    {
    }

private:

};

void CTsEncoderInit()
{
    static CTsEncoder::CFactory instance;
}

void CTsEncoderUninit()
{
}

