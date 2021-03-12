#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "IRtspHandler.h"
#include "ITcpSession.h"
#include "IPycaiLogger.h"

class CTcpSession : public ITcpSession
{
public:
    class CFactory : public ITcpSession::IFactory
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
            return "CTcpSession";
        }

        IUnknowObject* Create()
        {
            return new CTcpSession();
        }
    };

    CTcpSession() {}
    virtual ~CTcpSession() {}

    const char* GetConfig(const char*) const override
    {
        return "";
    }

    bool SetConfig(const char* key, const char* value) override
    {
        if (std::string(key) == "local.ip") { localIp_ = value; return true; }
        if (std::string(key) == "local.port") { localPort_ = (int)(unsigned long)value; return true; }
        if (std::string(key) == "peer.ip") { peerIp_ = value; return true; }
        if (std::string(key) == "peer.port") { peerPort_ = (int)(unsigned long)value; return true; }
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool Run(int skt) override
    {
        skt_ = skt;

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

private:

    static void* entry(void* arg)
    {
        if (arg == nullptr) {
            PYCAI_ERROR("arg is null.");
            return 0;
        }
        CTcpSession* s = (CTcpSession*)arg;
        s->Loop();
        s->Destroy();
        return 0;
    }

    void Loop()
    {
        while(true) {
            std::string req = ReadOneRequest();
            if (req.empty()) {
                PYCAI_ERROR("request is null.");
                break;
            }
            if (!rtsp_) {
                rtsp_ = CreateComponentObject<IRtspHandler>("CRtspHandler");
                if (!rtsp_) {
                    PYCAI_ERROR("can not create rtsp handler");
                    break;
                }
                rtsp_->SetConfig("local.ip", localIp_.c_str());
                rtsp_->SetConfig("peer.ip", peerIp_.c_str());
                rtsp_->SetConfig("local.port", (char*)(unsigned long)localPort_);
                rtsp_->SetConfig("peer.port", (char*)(unsigned long)peerPort_);
            }
            std::string resp = rtsp_->Handle(req);
            if (resp.empty()) {
                PYCAI_ERROR("respone is empty");
                break;
            }
            if (!SendOneResponse(resp)) {
                PYCAI_ERROR("send response fail.");
                break;
            }
        }
    }

    std::string ReadOneRequest()
    {
        char buf[100 * 1024] = { 0 };
        ssize_t ret = recv(skt_, buf, sizeof(buf), 0);
        int err = errno;
        if (ret <= 0) {
            PYCAI_ERROR("recv fail, ret[%d], msg[%s], socket[%d]", (int)ret, strerror(err), skt_);
            return "";
        }
        if (ret >= sizeof(buf)) {
            PYCAI_ERROR("recv fail, request msg too large, socket[%d]", skt_);
            return "";
        }
        return std::string(buf);
    }

    bool SendOneResponse(const std::string& resp)
    {
        ssize_t ret = send(skt_, resp.c_str(), resp.size(), 0);
        int err = errno;
        if (ret <= 0) {
            PYCAI_ERROR("send fail, ret[%d], msg[%s], socket[%d]", (int)ret, strerror(err), skt_);
            return false;
        }
        return true;
    }

    std::string localIp_;
    std::string peerIp_;
    int localPort_ = 0;
    int peerPort_ = 0;
    int skt_ = -1;
    IRtspHandler* rtsp_ = 0;
};

void CTcpSessionInit()
{
    static CTcpSession::CFactory instance;
}

void CTcpSessionUninit()
{
}

