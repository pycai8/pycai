#include <string>
#include <string.h>
#include <errno.h>

#include "ITsEncoder.h"
#include "IPycaiLogger.h"

class TsHeader {
public:
    /* byte 0 */
    uint32_t sync_byte:8;
    /* byte 1 ~ 2 */
    uint32_t transport_error_indicator:1;
    uint32_t payload_unit_start_indicator:1;
    uint32_t transport_priority:1;
    uint32_t pid:13;
    /* byte 3 */
    uint32_t transport_scrambling_control:2;
    uint32_t adaptation_field_control:2;
    uint32_t continuity_count:4;
   
private:
    void Init()
    {
        // fixed 0x47
        sync_byte = 0x47;
        // 0: not bit error; 1: has bit error;
        transport_error_indicator = 0;
        // 0: no adjust byte; 1: has one adjust byte;
        payload_unit_start_indicator = 0;
        // 0: normal priority; 1: high priority;
        transport_priority = 0;
        // type of ts package
        pid = 0;
        // 0: no scrambling; other: has scrambling;
        transport_scrambling_control = 0;
        // 0b01: payload only; 0b10: adaptation only; 0b11: adaptation and payload
        adaptation_field_control = 1;
        // count, like 0,1,...,15,0,1,...15,0... for ts packages.
        continuity_count = 0;
    }

public:
    TsHeader(const std::string& type)
    {
        Init();
        if (type == "pat") {
            payload_unit_start_indicator = 1;
            transport_priority = 1;
        } else if (type == "pmt") {
            payload_unit_start_indicator = 1;
            transport_priority = 1;
            pid = 0x81;
        }
    }

    TsHeader(const std::string& type, bool padding, int cc)
    {
        Init();
        if (type == "pes") {
            pid = 0x810;
            adaptation_field_control = (padding ? 3 : 1);
            continuity_count = cc;
        }
    }
};

static uint32_t crc32table[256] = {
    0x00000000, 0xB71DC104, 0x6E3B8209, 0xD926430D, 0xDC760413, 0x6B6BC517,
    0xB24D861A, 0x0550471E, 0xB8ED0826, 0x0FF0C922, 0xD6D68A2F, 0x61CB4B2B,
    0x649B0C35, 0xD386CD31, 0x0AA08E3C, 0xBDBD4F38, 0x70DB114C, 0xC7C6D048,
    0x1EE09345, 0xA9FD5241, 0xACAD155F, 0x1BB0D45B, 0xC2969756, 0x758B5652,
    0xC836196A, 0x7F2BD86E, 0xA60D9B63, 0x11105A67, 0x14401D79, 0xA35DDC7D,
    0x7A7B9F70, 0xCD665E74, 0xE0B62398, 0x57ABE29C, 0x8E8DA191, 0x39906095,
    0x3CC0278B, 0x8BDDE68F, 0x52FBA582, 0xE5E66486, 0x585B2BBE, 0xEF46EABA,
    0x3660A9B7, 0x817D68B3, 0x842D2FAD, 0x3330EEA9, 0xEA16ADA4, 0x5D0B6CA0,
    0x906D32D4, 0x2770F3D0, 0xFE56B0DD, 0x494B71D9, 0x4C1B36C7, 0xFB06F7C3,
    0x2220B4CE, 0x953D75CA, 0x28803AF2, 0x9F9DFBF6, 0x46BBB8FB, 0xF1A679FF,
    0xF4F63EE1, 0x43EBFFE5, 0x9ACDBCE8, 0x2DD07DEC, 0x77708634, 0xC06D4730,
    0x194B043D, 0xAE56C539, 0xAB068227, 0x1C1B4323, 0xC53D002E, 0x7220C12A,
    0xCF9D8E12, 0x78804F16, 0xA1A60C1B, 0x16BBCD1F, 0x13EB8A01, 0xA4F64B05,
    0x7DD00808, 0xCACDC90C, 0x07AB9778, 0xB0B6567C, 0x69901571, 0xDE8DD475,
    0xDBDD936B, 0x6CC0526F, 0xB5E61162, 0x02FBD066, 0xBF469F5E, 0x085B5E5A,
    0xD17D1D57, 0x6660DC53, 0x63309B4D, 0xD42D5A49, 0x0D0B1944, 0xBA16D840,
    0x97C6A5AC, 0x20DB64A8, 0xF9FD27A5, 0x4EE0E6A1, 0x4BB0A1BF, 0xFCAD60BB,
    0x258B23B6, 0x9296E2B2, 0x2F2BAD8A, 0x98366C8E, 0x41102F83, 0xF60DEE87,
    0xF35DA999, 0x4440689D, 0x9D662B90, 0x2A7BEA94, 0xE71DB4E0, 0x500075E4,
    0x892636E9, 0x3E3BF7ED, 0x3B6BB0F3, 0x8C7671F7, 0x555032FA, 0xE24DF3FE,
    0x5FF0BCC6, 0xE8ED7DC2, 0x31CB3ECF, 0x86D6FFCB, 0x8386B8D5, 0x349B79D1,
    0xEDBD3ADC, 0x5AA0FBD8, 0xEEE00C69, 0x59FDCD6D, 0x80DB8E60, 0x37C64F64,
    0x3296087A, 0x858BC97E, 0x5CAD8A73, 0xEBB04B77, 0x560D044F, 0xE110C54B,
    0x38368646, 0x8F2B4742, 0x8A7B005C, 0x3D66C158, 0xE4408255, 0x535D4351,
    0x9E3B1D25, 0x2926DC21, 0xF0009F2C, 0x471D5E28, 0x424D1936, 0xF550D832,
    0x2C769B3F, 0x9B6B5A3B, 0x26D61503, 0x91CBD407, 0x48ED970A, 0xFFF0560E,
    0xFAA01110, 0x4DBDD014, 0x949B9319, 0x2386521D, 0x0E562FF1, 0xB94BEEF5,
    0x606DADF8, 0xD7706CFC, 0xD2202BE2, 0x653DEAE6, 0xBC1BA9EB, 0x0B0668EF,
    0xB6BB27D7, 0x01A6E6D3, 0xD880A5DE, 0x6F9D64DA, 0x6ACD23C4, 0xDDD0E2C0,
    0x04F6A1CD, 0xB3EB60C9, 0x7E8D3EBD, 0xC990FFB9, 0x10B6BCB4, 0xA7AB7DB0,
    0xA2FB3AAE, 0x15E6FBAA, 0xCCC0B8A7, 0x7BDD79A3, 0xC660369B, 0x717DF79F,
    0xA85BB492, 0x1F467596, 0x1A163288, 0xAD0BF38C, 0x742DB081, 0xC3307185,
    0x99908A5D, 0x2E8D4B59, 0xF7AB0854, 0x40B6C950, 0x45E68E4E, 0xF2FB4F4A,
    0x2BDD0C47, 0x9CC0CD43, 0x217D827B, 0x9660437F, 0x4F460072, 0xF85BC176,
    0xFD0B8668, 0x4A16476C, 0x93300461, 0x242DC565, 0xE94B9B11, 0x5E565A15,
    0x87701918, 0x306DD81C, 0x353D9F02, 0x82205E06, 0x5B061D0B, 0xEC1BDC0F,
    0x51A69337, 0xE6BB5233, 0x3F9D113E, 0x8880D03A, 0x8DD09724, 0x3ACD5620,
    0xE3EB152D, 0x54F6D429, 0x7926A9C5, 0xCE3B68C1, 0x171D2BCC, 0xA000EAC8,
    0xA550ADD6, 0x124D6CD2, 0xCB6B2FDF, 0x7C76EEDB, 0xC1CBA1E3, 0x76D660E7,
    0xAFF023EA, 0x18EDE2EE, 0x1DBDA5F0, 0xAAA064F4, 0x738627F9, 0xC49BE6FD,
    0x09FDB889, 0xBEE0798D, 0x67C63A80, 0xD0DBFB84, 0xD58BBC9A, 0x62967D9E,
    0xBBB03E93, 0x0CADFF97, 0xB110B0AF, 0x060D71AB, 0xDF2B32A6, 0x6836F3A2,
    0x6D66B4BC, 0xDA7B75B8, 0x035D36B5, 0xB440F7B1
};

uint32_t CalcCrc(uint8_t* buffer, uint32_t size)
{
    uint32_t crc = 0xffffffff;
    for (uint32_t i = 0; i < size; i++) {
        crc = crc32table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ;
}

class PatInfo {
public:
    /* byte 0 */
    uint32_t table_id:8; 
    /* byte 1 ~ 2 */
    uint32_t section_syntax_indicator:1; 
    uint32_t zero:1; 
    uint32_t reserved1:2; 
    uint32_t section_length:12;
    /* byte 3 ~ 4 */
    uint32_t transport_stream_id:16;
    /* byte 5 */
    uint32_t reserved2:2;
    uint32_t version_number:5;
    uint32_t current_next_indicator:1;
    /* byte 6 */
    uint32_t section_number:8; 
    /* byte 7 */
    uint32_t last_section_number:8;
    /* byte 8 ~ 9 */
    uint32_t program_number:16;
    /* byte 10 ~ 11 */
    uint32_t reserved3:3;
    uint32_t program_map_pid:13;
    /* byte 12 ~ 15 */
    uint32_t crc32;

    PatInfo()
    {
        // fixed 0
        table_id = 0;
        // fixed 1
        section_syntax_indicator = 1;
        // fixed 0
        zero = 0;
        // fixed 0b11
        reserved1 = 3;
        // fixed 13, the usefull bytes count after this
        section_length = 13;
        // fixed 1
        transport_stream_id = 1;
        // fixed 0b11
        reserved2 = 3;
        // let 0
        version_number = 0;
        // let 1, this usefull
        current_next_indicator = 1;
        // let 0
        section_number = 0;
        // let 0
        last_section_number = 0;
        // let 1
        program_number = 1;
        // fixed 0b111
        reserved3 = 7;
        // let 0x81
        program_map_pid = 0x81;
        // calc crc
        crc32 = CalcCrc((uint8_t*)this, sizeof(*this) - 4);
    }
};

class PmtInfo {
public:
    uint32_t table_id:8;

    uint32_t section_syntax_indicator:1;
    uint32_t zero:1;  
    uint32_t reserved1:2; 
    uint32_t section_length:12;  

    uint32_t program_number:16;  
    
    uint32_t reserved2:2; 
    uint32_t version_number:5; 
    unsigned current_next_indicator:1;  
    
    uint32_t section_number:8;  
    
    uint32_t last_section_number:8; 
    
    uint32_t reserved3:3;  
    uint32_t pcr_pid:13;  
    
    uint32_t reserved4:4;  
    uint32_t program_info_length:12; 
    
    uint32_t stream_type:8;

    uint32_t reserved5:3;
    uint32_t elementary_pid:13;

    uint32_t reserved6:4;
    uint32_t es_info_length:12;

    uint32_t crc32:32;   

    PmtInfo()
    {
        // fixed 2
        table_id = 2;
        // fixed 1
        section_syntax_indicator = 1;
        // fixed 0
        zero = 0;
        // fixed 0b11
        reserved1 = 3;
        // fixed 18, the count of the after usefull bytes.
        section_length = 18;
        // fixed 1 which is the same with pat
        program_number = 1;
        // fixed 0b11
        reserved2 = 3;
        // fixed 0
        version_number = 0;
        // 1 usefull pmt
        current_next_indicator = 1;
        // fixed 0
        section_number = 0;
        // fixed 0
        last_section_number = 0;
        // fixed 0b111
        reserved3 = 7;
        // the pid of ts package that containing pcr
        pcr_pid = 0x810;
        // fixed 0b1111
        reserved4 = 15;
        // fixed 0, no other info
        program_info_length = 0;
        // 0x1b for h264
        stream_type = 0x1b;
        // fixe 0b111
        reserved5 = 7;
        // let pid of h264 es to be 0x810
        elementary_pid = 0x810;
        // fixed 0b1111
        reserved6 = 15;
        // no es info
        es_info_length = 0;
        crc32 = CalcCrc((uint8_t*)this, sizeof(this) - 4); 
    }
};

class PesHeader {
public:
    uint32_t package_start_code_prefix:24;

    uint32_t stream_id:8;
    
    uint32_t pes_package_length:16;

    uint32_t bit_1_0:2;
    uint32_t pes_scrambling_control:2;
    uint32_t pes_priority:1;
    uint32_t data_alignment_indicator:1;
    uint32_t copyright:1;
    uint32_t original_or_copy:1;

    uint32_t pts_flag:1;
    uint32_t dts_flag:1;
    uint32_t escr_flag:1;
    uint32_t es_rate_flag:1;
    uint32_t dsm_trick_mode_flag:1;
    uint32_t additional_copy_info_flag:1;
    uint32_t pes_crc_flag:1;
    uint32_t pes_extension_flag:1;

    uint32_t pes_header_data_length:8;

    uint32_t pts_start_code:4;
    uint32_t pts_32_30:3;
    uint32_t pts_marker1:1;
    uint32_t pts_29_15:15;
    uint32_t pts_marker2:1;
    uint32_t pts_14_0:15;
    uint32_t pts_marker3:1;

    PesHeader(uint64_t timestamp)
    {
        // fixed 0x000001
        package_start_code_prefix = 0x1;
        stream_id = 0xe0; // h264 video
        pes_package_length = 0; // auto calc
        bit_1_0 = 2; // 0b10
        pes_scrambling_control = 0; // no encryption
        pes_priority = 0; // normal priority
        data_alignment_indicator = 0;
        copyright = 0;
        original_or_copy = 0;
        pts_flag = 1;
        dts_flag = 0;
        escr_flag = 0;
        es_rate_flag = 0;
        dsm_trick_mode_flag = 0;
        additional_copy_info_flag = 0;
        pes_crc_flag = 0;
        pes_extension_flag = 0;
        pes_header_data_length = 5; // 5 bytes for pts
        pts_start_code = 2; // 0b0010
        pts_32_30 = (timestamp >> 30) & 0x3; // 0x3 = 0b111
        pts_marker1 = 1;
        pts_29_15 = (timestamp >> 15) & 0x7fff; //0b 111 1111 1111 1111
        pts_marker2 = 1;
        pts_14_0 = timestamp & 0x7fff;
        pts_marker3 = 1;
    }
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
        FILE* fpSrc = fopen(h264File, "r");
        int err = errno;
        if (fpSrc == 0) {
            PYCAI_ERROR("open 264 file fail[%s]", strerror(err));
            return false;
        }

        FILE* fpDest = fopen(tsFile, "w+");
        err = errno;
        if (fpDest == 0) {
            PYCAI_ERROR("open ts file fail[%s]", strerror(err));
            fclose(fpSrc);
            fpSrc = 0;
            return false;
        }

        if (!WritePat(fpDest)) {
            PYCAI_ERROR("write pat fail.");
        }
        if (!WritePmt(fpDest)) {
            PYCAI_ERROR("write pmt fail");
        }
        while (true) {
            char buf[900 * 1024] = { 0 };
            char* inBuf = buf + sizeof(PesHeader);
            int inLen = sizeof(buf) - sizeof(PesHeader);
            char* outBuf = 0;
            int outLen = 0;
            if (!ReadOneFrame(fpSrc, inBuf, inLen, &outBuf, &outLen)) {
                PYCAI_ERROR("read one frame fail.");
                break;
            }
            uint8_t naluHeader = outBuf[0];
            uint8_t naluType = naluHeader & 0x1f;
            if (naluType != 7 && naluType != 8) { // not sps and not pps
                timestamp_ += 90000 / 25; // 90000 clock rate, 25 frame rate.
            }
            if (!WriteOneFrame(fpDest, outBuf, outLen)) {
                PYCAI_ERROR("write one frame fail");
                break;
            }
        }

        fclose(fpSrc);
        fpSrc = 0;
        fclose(fpDest);
        fpDest = 0;
        return true;
    }

    bool WritePat(FILE* fp)
    {
        TsHeader hdr("pat");
        if (!WriteBuf(fp, (char*)&hdr, sizeof(hdr))) return false;

        uint8_t adjustByte = 0;
        if (!WriteBuf(fp, (char*)&adjustByte, 1)) return false;

        PatInfo info;
        if (!WriteBuf(fp, (char*)&info, sizeof(info))) return false;

        uint8_t padding[188] = { 0xff };
        if (!WriteBuf(fp, (char*)padding, 188 - sizeof(hdr) - 1 - sizeof(info))) return false;

        return true;
    }

    bool WritePmt(FILE* fp)
    {
        TsHeader hdr("pmt");
        if (!WriteBuf(fp, (char*)&hdr, sizeof(hdr))) return false;

        uint8_t adjustByte = 0;
        if (!WriteBuf(fp, (char*)&adjustByte, 1)) return false;

        PmtInfo info;
        if (!WriteBuf(fp, (char*)&info, sizeof(info))) return false;

        uint8_t padding[188] = { 0xff };
        if (!WriteBuf(fp, (char*)padding, 188 - sizeof(hdr) - 1 - sizeof(info))) return false;

        return true;
    }

    bool ReadOneFrame(FILE* fp, char* inBuf, int inLen, char** outBuf, int* outLen)
    {
        size_t sz = fread(inBuf, 1, inLen, fp);
        int err = errno;
        if (sz <= 0) {
            PYCAI_ERROR("read file fail, inLen[%d], ret[%d], msg[%s]", inLen, (int)sz, strerror(err));
            return false;
        }

        // frame start with "0 0 1" or "0 0 0 1"
        int start = -1;
        int end = -1;
        for (int i = 0; i < sz; ++i) {
            if (i + 2 < sz && inBuf[i + 0] == 0 && inBuf[i + 1] == 0 && inBuf[i + 2] == 1) {
                if (start == -1) start = i + 3;
                else if (end == -1) { end = i; break; }
            } else if (i + 3 < sz && inBuf[i + 0] == 0 && inBuf[i + 1] == 0 && inBuf[i + 2] == 0 && inBuf[i + 3] == 1) {
                if (start == -1) start = i + 4;
                else if (end = -1) { end = i; break; }
            }
        }
        if (start == -1) return false;
        *outBuf = inBuf + start;
        if (end == -1) {
            *outLen = sz - start;
            fseek(fp, 0, SEEK_END); // to end position
        } else {
            *outLen = end - start;
            fseek(fp, end - sz, SEEK_CUR);
        }
        return true;
    }

    bool WriteOneFrame(FILE* fp, char* buf, int len)
    {
        char* pesBuf = buf - sizeof(PesHeader);
        int pesLen = len + sizeof(PesHeader);
        PesHeader pesHdr(timestamp_);
        memcpy(pesBuf, &pesHdr, sizeof(pesHdr));

        int pesClipCount = pesLen / 184;
        if (pesLen % 184 != 0) pesClipCount++;
        int cc = 0;
        for (int i = 0; i < pesClipCount; ++i) {
            bool padding = (pesLen % 184 != 0 && i == pesClipCount - 1);
            TsHeader hdr("pes", padding, cc);
            cc = (cc + 1) % 16;
            if (!WriteBuf(fp, (char*)&hdr, sizeof(hdr))) return false;

            if (padding) {
                uint8_t tmpBuf[188] = { 0xff };
                int tmpLen = 188 - sizeof(TsHeader) - pesLen % 184;
                if (!WriteBuf(fp, (char*)tmpBuf, tmpLen)) return false;
            }

            char* pesClipBuf = pesBuf + i * 184;
            int pesClipLen = (padding ? pesLen % 184 : 184);
            if (!WriteBuf(fp, pesClipBuf, pesClipLen)) return false;
        }

        return true;
    }

    bool WriteBuf(FILE* fp, char* buf, int len)
    {
        auto ret = fwrite(buf, 1, len, fp);
        int err = errno;
        if (ret <= 0) {
            PYCAI_ERROR("write fail, ret[%d], msg[%s]", (int)ret, strerror(err));
        }
        return ret > 0;
    } 

private:
    uint64_t timestamp_ = 0;
};

void CTsEncoderInit()
{
    static CTsEncoder::CFactory instance;
}

void CTsEncoderUninit()
{
}

