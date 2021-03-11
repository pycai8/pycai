#include <string>
#include <dlfcn.h>

#include "IMediaSession.h"
#include "IPycaiLogger.h"

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
        if (ret != 0) PYCAI_ERROR("pthread_attr_init fail, socket[%d], ret[%d]", skt_, ret);
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (ret != 0) PYCAI_ERROR("pthread_attr_setdetachstate fail, socket[%d], ret[%d]", skt_, ret);

        pthread_t th;
        int result = pthread_create(&th, &attr, entry, this);
        if (result != 0) PYCAI_ERROR("pthread_create fail, socket[%d], ret[%d]", skt_, result);
        ret = pthread_attr_destroy(&attr);
        if (ret != 0) PYCAI_ERROR("pthread_attr_destroy fail, socket[%d], ret[%d]", skt_, ret);
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

    }

    bool SendOneFrame(char* buf, int len)
    {
    }

    bool run_ = false;
    IUdpHelper* udp_ = 0;
    std::string file_ = "";
};

void CRtpSessionInit()
{
    static CRtpSession::CFactory instance;
}

void CRtpSessionUninit()
{
}

