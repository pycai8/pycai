#include <string>
#include <dlfcn.h>

#include "IMediaSession.h"
#include "IPycaiLogger.h"

class CRtcpSession : public IMediaSession
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
            return "CRtcpSession";
        }

        IUnknowObject* Create()
        {
            return new CRtcpSession();
        }
    };

    CRtcpSession() {}

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
        CRtcpSession* s = (CRtcpSession*)arg;
        s->Loop();
        return 0;
    }

    void Loop()
    {
        while (run_) {
            usleep(10000); // sleep 10ms
            if (!ReadPackage()) break;
        }
    }


    bool ReadPackage()
    {
        if (!udp_) {
            PYCAI_ERROR("udp helper is null.");
            return false;
        }

        char buf[100 * 1024] = { 0 };
        int len = udp_->Recv(buf, sizeof(buf));
        PYCAI_INFO("rtcp recv buf[%d]", len);
        return len > 0;
    }

    IUdpHelper* udp_ = 0;
    std::string file_ = "";
    bool run_ = false;
};

void CRtcpSessionInit()
{
    static CRtcpSession::CFactory instance;
}

void CRtcpSessionUninit()
{
}

