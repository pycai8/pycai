#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "IMediaSession.h"
#include "IPycaiLogger.h"

#define MTU 1400

typedef struct {
    /* byte 0 */
    uint8_t csrcCount:4;
    uint8_t extension:1;
    uint8_t padding:1;
    uint8_t version:2;

    /* byte 1 */
    uint8_t payloadType:7;
    uint8_t marker:1;

    /* byte 2 ~ 3 */
    uint16_t seq;

    /* byte 4 - 7 */
    uint32_t timestamp;

    /* byte 8 ~ 11 */
    uint32_t ssrc;
} RtpHeader;

class CRtpSession : public IMediaSession
{
public:
    class CFactory : public IMediaSession::IFactory
    {
    public:
        CFactory()
        {
            RegisterFactory();
        }
        ~CFactory()
        {
            UnregisterFactory();
        }

        const char* GetClassId() const override
        {
            return "CRtpSession";
        }

        IUnknowObject* Create()
        {
            return new CRtpSession();
        }
    };

    const char* GetConfig(const char*) const override
    {
        return "";
    }

    bool SetConfig(const char* key, const char* value) override
    {
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool Init(IUdpHelper* value) override
    {
        udp_ = value;
        return true;
    }

    bool Start(const char* file) override
    {
        file_ = file;
        run_ = true;
        pthread_attr_t attr;
        int ret = pthread_attr_init(&attr);
        if (ret != 0) PYCAI_ERROR("pthread_attr_init fail, ret[%d]", ret);
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (ret != 0) PYCAI_ERROR("pthread_attr_setdetachstate fail, ret[%d]", ret);

        pthread_t th;
        int result = pthread_create(&th, &attr, entry, this);
        if (result != 0) PYCAI_ERROR("pthread_create fail, ret[%d]", result);
        ret = pthread_attr_destroy(&attr);
        if (ret != 0) PYCAI_ERROR("pthread_attr_destroy fail, ret[%d]", ret);
        return (result == 0); // equal to thread create success.
    }

    bool Stop() override
    {
        run_ = false;
        return true;
    }

private:

    static void* entry(void* arg)
    {
        if (arg == nullptr) {
            PYCAI_ERROR("arg is null.");
            return 0;
        }
        CRtpSession* s = (CRtpSession*)arg;
        s->Loop();
        return 0;
    }

    void Loop()
    {
        PYCAI_INFO("enter rtp session loop.");
        char buf[1024] = { 0 };
    int len = udp_->Recv(buf, sizeof(buf));
    PYCAI_INFO("rtp session recv[%d]", len);

        FILE* fp = fopen(file_.c_str(), "r");
        int err = errno;
        if (!fp) {
            PYCAI_ERROR("file[%s] open fail[%s].", file_.c_str(), strerror(err));
            return;
        }

        while(run_) {
            usleep(30000); // sleep 30ms
            if (!HandleNextFrame(fp)) break;
        }

        PYCAI_INFO("one rtp session stoped");
        fclose(fp);
        fp = 0;
    }

    bool HandleNextFrame(FILE* fp)
    {
        char buf[500 * 1024] = { 0 };
        int len = sizeof(buf);
        int hdrLen = 12 + 2; // 12 is rtp header, 2 is clip header.
        char* inBuf = buf + hdrLen;
        int inLen = len - hdrLen;
        char* outBuf = 0;
        int outLen = 0;
        if (!ReadOneFrame(fp, inBuf, inLen, &outBuf, &outLen)) {
            PYCAI_ERROR("read frame fail.");
            return false;
        }

        if (!SendOneFrame(outBuf, outLen)) {
            PYCAI_ERROR("send frame fail.");
            return false;
        }

        return true;
    }

    bool ReadOneFrame(FILE* fp, char* inBuf, int inLen, char** outBuf, int* outLen)
    {
        size_t sz = fread(inBuf, 1, inLen, fp);
        int err = errno;
        if (sz <= 0) {
            PYCAI_ERROR("read file[%s] fail.", file_.c_str());
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
            fseek(fp, 0, SEEK_SET); // to start position
        } else {
            *outLen = end - start;
            fseek(fp, end - sz, SEEK_CUR);
        }
        return true;
    }

    bool SendOneFrame(char* buf, int len)
    {
        uint8_t naluHeader = buf[0];
        uint8_t naluType = naluHeader & 0x1F;
        if (naluType != 7 && naluType != 8) {
            // is not SPS and PPS => add timestamp
            timestamp_ += 90000 / 25; // 90000 is ClockRate, 25 is framerate
        }

        if (len <= MTU) {
            FillRtpHeader((RtpHeader*)(buf - 12)); // rtp header is 12 bytes
            udp_->Send(buf - 12, len + 12);
            return true;
        }

        int clipCount = len / MTU;
        if (len % MTU != 0) clipCount++;
	buf++;
        for (int i = 0; i < clipCount; ++i) {
            char* clipBuf = buf + i * MTU;
            int clipLen = 0;
            if (len % MTU == 0) clipLen = MTU;
            else if (i != clipCount - 1) clipLen = MTU;
            else clipLen = len % MTU;

            FillRtpHeader((RtpHeader*)(clipBuf - 12 - 2)); // 12 rtp header, 2 clip header
            *(clipBuf - 2) = (naluHeader & 0x60 ) | 28; // 0x60 get old important, 28 is clip
            *(clipBuf - 1) = (naluHeader & 0x1F); // 0x1F get old payload type
            if (i == 0) *(clipBuf - 1) |= 0x80; // 0x80 clip start
            if (i == clipCount - 1) *(clipBuf - 1) |= 0x40; // 0x40 clip end 
            udp_->Send(clipBuf - 12 - 2, clipLen + 12 + 2); // 12 rtp header, 2 clip header
            usleep(10000); // sleep 10ms
        }
        return true;
    }

    void FillRtpHeader(RtpHeader* hdr)
    {
        hdr->version = 2; // V=2
        hdr->padding = 0; // no padding
        hdr->extension = 0; // no extension
        hdr->csrcCount = 0; // no extension, no csrc
        hdr->marker = 0;
        hdr->payloadType = 96; // H264
        hdr->seq = htons(seq_++);
        hdr->timestamp = htonl(timestamp_);
        hdr->ssrc = htonl(ssrc_);
    }

    bool run_ = false;
    IUdpHelper* udp_ = 0;
    std::string file_ = "";
    uint16_t seq_ = 1;
    uint32_t timestamp_ = 0;
    uint32_t ssrc_ = rand();
};

void CRtpSessionInit()
{
    static CRtpSession::CFactory instance;
}

void CRtpSessionUninit()
{
}

