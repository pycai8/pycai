#include <string>
#include <string.h>
#include <errno.h>
#include <vector>

#include "IMjpgEncoder.h"
#include "IPycaiLogger.h"

class CMjpgEncoder : public IMjpgEncoder
{
public:
    class CFactory : public IMjpgEncoder::IFactory
    {
    public:
        CFactory()
        {
            RegisterFactory();
            avcodec_register_all();
            av_register_all();
            m_decCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
            m_encCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
        }
        virtual ~CFactory()
        {
            UnregisterFactory();
        }

        const char* GetClassId() const override
        {
            return "CMjpgEncoder";
        }

        IUnknowObject* Create()
        {
            return new CMjpgEncoder(m_decCodec, m_encCodec);
        }

    private:
        AVCodec* m_decCodec;
        AVCodec* m_encCodec;
    };

    CMjpgEncoder(AVCodec* dec, AVCodec* enc) 
    {
        m_decCodec = dec;
        m_encCodec = enc;
        m_decContext = avcodec_alloc_context3(m_decCodec);
        m_encContext = avcodec_alloc_context3(m_encCodec);
        avcodec_open2(m_decContext, m_decCodec, 0);
        avcodec_open2(m_encContext, m_encCodec, 0);
    }
    virtual ~CMjpgEncoder() 
    {
        for (int i = 0; i < m_vecOut.size(); i++)
        {
            av_packet_unref(m_vecOut[i]);
            av_packet_free(m_vecOut[i]);
        }
        m_vecOut.clear();
        avcodec_free_context(m_encContext);
        avcodec_free_context(m_decContext);
    }

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

    bool H264ToMjpg(const char* h264File) override
    {   
        FILE* fpSrc = fopen(h264File, "r");
        int err = errno;
        if (fpSrc == 0) {
            PYCAI_ERROR("open 264 file fail[%s]", strerror(err));
            return false;
        }

        while (true) {
            uint8_t buf[900 * 1024] = { 0 };
            uint8_t* inBuf = buf;
            int inLen = sizeof(buf);
            uint8_t* outBuf = 0;
            int outLen = 0;
            if (!ReadOneFrame(fpSrc, inBuf, inLen, &outBuf, &outLen)) {
                PYCAI_ERROR("read one frame fail.");
                break;
            }
            uint8_t naluHeader = outBuf[0];
            uint8_t naluType = naluHeader & 0x1f;
            if (naluType != 7 && naluType != 8) { // not sps and not pps
                timestamp_ += 90000 / 25; // 90000 clock rate, 25 frame rate
                pcr_ += 27000000ull / 25; // 27m clock rate, 25 frame rate
            }
            if (!WriteOneFrame(outBuf, outLen, naluType)) {
                PYCAI_ERROR("write one frame fail");
                break;
            }
        }

        fclose(fpSrc);
        fpSrc = 0;
        return true;
    }
    
    int GetJpgCount() const
    {
        return m_vecOut.size();
    }

    virtual int GetJpgLen(int index) const
    {
        return m_vecOut[index]->size;
    }

    virtual const char* GetJpgData(int index) const
    {
        return m_vecOut[index]->data;
    }

private:

    bool ReadOneFrame(FILE* fp, uint8_t* inBuf, int inLen, uint8_t** outBuf, int* outLen)
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

    bool WriteOneFrame(uint8_t* buf, int len, uint8_t naluType)
    {
        AVPacket* pkt = av_packet_alloc();
        av_packet_from_data(pkt, buf, len);
        avcodec_send_packet(m_decContext, pkt);
        AVFrame* frame = av_frame_alloc();
        while (avcodec_receive_frame(m_decContext, frame) >= 0)
        {
            AVPacket* out = av_packet_alloc();
            avcodec_send_frame(m_encContext, frame);
            while (avcodec_receive_packet(m_encContext, out) >= 0)
            {
                m_vecOut.push_back(av_packet_clone(out));
                av_packet_unref(out);
            }
            av_packet_unref(out);
            av_packet_free(out);
            av_frame_unref(frame);
        }
        av_frame_unref(frame);
        av_frame_free(frame);
        av_packet_unref(pkt);
        av_packet_free(pkt);
        return true;
    }

    uint64_t timestamp_ = 0;
    uint64_t pcr_ = 0;
    int cc_ = 0;
    AVCodec* m_decCodec;
    AVCodec* m_encCodec;
    AVCodecContext* m_decContext;
    AVCodecContext* m_encContext;
    std::vector<AVPacket*> m_vecOut;
};

void CMjpgEncoderInit()
{
    static CMjpgEncoder::CFactory instance;
}

void CMjpgEncoderUninit()
{
}

