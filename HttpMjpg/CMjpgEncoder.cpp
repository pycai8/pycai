#include <string>
#include <string.h>
#include <errno.h>
#include <vector>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

#include "IMjpgEncoder.h"
#include "IPycaiLogger.h"

//LEVEL_DEBUG = 0, LEVEL_INFO, LEVEL_WARN, LEVEL_ERROR, LEVEL_FATAL
static int cvtLogLevel(int level)
{
    switch (level)
    {
        case AV_LOG_FATAL     : return LEVEL_FATAL;
        case AV_LOG_ERROR     : return LEVEL_ERROR;
        case AV_LOG_WARNING   : return LEVEL_WARN;
        case AV_LOG_INFO      : return LEVEL_INFO;
        case AV_LOG_DEBUG     : return LEVEL_DEBUG;
        case AV_LOG_TRACE     : return LEVEL_DEBUG;
        default               : return LEVEL_DEBUG;
    }
    return LEVEL_INFO;
}

static void ffmpeg_logoutput(void* ptr, int level, const char* fmt, va_list args){
    if (level == AV_LOG_DEBUG) return;

    char str[1024] = { 0 };
    vsnprintf(str, sizeof(str) - 1, fmt, args);
    str[strlen(str) - 1] = 0;
    PYCAI_LOGGER(cvtLogLevel(level), str); 
}

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
            m_decCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
            if (!m_decCodec) PYCAI_ERROR("find AVCodec(AV_CODEC_ID_H264) decoder fail");
            m_encCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
            if (!m_encCodec) PYCAI_ERROR("find AVCodec(AV_CODEC_ID_MJPEG) encoder fail.");
            av_log_set_callback(ffmpeg_logoutput);
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
            if (!m_decCodec || !m_encCodec)
            {
                PYCAI_ERROR("H264 deocder or MJPEG encoder is null, can not create CMjpegEncoder");
                return 0;
            }
            return new CMjpgEncoder(m_decCodec, m_encCodec);
        }

    private:
        AVCodec* m_decCodec;
        AVCodec* m_encCodec;
    };

    CMjpgEncoder(AVCodec* dec, AVCodec* enc) 
    {
        m_init = false;
        m_decCodec = dec;
        m_encCodec = enc;
        m_decContext = avcodec_alloc_context3(m_decCodec);
        m_encContext = avcodec_alloc_context3(m_encCodec);
        if (!m_decContext || !m_encContext)
        {
            PYCAI_ERROR("decContext or encContext is null");
            return;
        }

        int ret = avcodec_open2(m_decContext, m_decCodec, 0);
        if (ret != 0)
        {
            PYCAI_ERROR("open decoder context fail, return[%d]", ret);
            return;
        }

        /* put sample parameters */
        m_encContext->bit_rate = 500000;
        /* resolution must be a multiple of two */
        m_encContext->width = 960;
        m_encContext->height = 544;
        /* frames per second */
        m_encContext->time_base = (AVRational){1, 25};
        m_encContext->framerate = (AVRational){25, 1};
        /* pixel format */
        m_encContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
        
        ret = avcodec_open2(m_encContext, m_encCodec, 0);
        if (ret != 0)
        {
            PYCAI_ERROR("open encoder context fail, return[%d]", ret);
            return;
        }
        m_init = true;
    }
    virtual ~CMjpgEncoder() 
    {
        for (int i = 0; i < m_vecOut.size(); i++)
        {
            av_packet_free(&m_vecOut[i]);
        }
        m_vecOut.clear();
        avcodec_free_context(&m_encContext);
        avcodec_free_context(&m_decContext);
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
        if (!m_init)
        {
            PYCAI_ERROR("have not init");
            return false;
        }
 
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
        return (const char*) m_vecOut[index]->data;
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
                if (start == -1) { start = i; i += 2; }
                else if (end == -1) { end = i; break; }
            } else if (i + 3 < sz && inBuf[i + 0] == 0 && inBuf[i + 1] == 0 && inBuf[i + 2] == 0 && inBuf[i + 3] == 1) {
                if (start == -1) { start = i; i += 3; }
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
        if (!pkt)
        {
            PYCAI_ERROR("av packet alloc fail");
            return false;
        }

        int ret =av_packet_from_data(pkt, buf, len);
        if (ret != 0) 
        {
            PYCAI_ERROR("av packet from data fail, return[%d]", ret);
            av_free(pkt);
            return false;
        }

        ret = avcodec_send_packet(m_decContext, pkt);
        if (ret != 0)
        {
            PYCAI_ERROR("send packet fail, return[%d]", ret);
            av_free(pkt);
            return true;
        }

        AVFrame* frame = av_frame_alloc();
        if (!frame)
        {
            PYCAI_ERROR("av frame alloc fail");
            av_free(pkt);
            return false;
        }

        while (avcodec_receive_frame(m_decContext, frame) == 0)
        {
            //PYCAI_DEBUG("decode one frame, width[%d], height[%d], format[%d]", frame->width, frame->height, frame->format);

            ret = avcodec_send_frame(m_encContext, frame);
            if (ret != 0)
            {
                PYCAI_ERROR("avcodec send frame fail, return[%d]", ret);
                break;
            }

            AVPacket* out = av_packet_alloc();
            if (!out)
            {
                PYCAI_ERROR("av packet alloc fail");
                break;
            }

            while (avcodec_receive_packet(m_encContext, out) == 0)
            {
                //PYCAI_DEBUG("encodec one frame, pts[%d], dts[%d], size[%d]", out->pts, out->dts, out->size);
                AVPacket* tmp = av_packet_clone(out);
                if (tmp) m_vecOut.push_back(tmp);
                else PYCAI_WARN("av packet clone fail");
            }

            av_packet_free(&out);
        }

        av_frame_free(&frame);
        av_free(pkt);
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
    bool m_init;
};

void CMjpgEncoderInit()
{
    static CMjpgEncoder::CFactory instance;
}

void CMjpgEncoderUninit()
{
}

