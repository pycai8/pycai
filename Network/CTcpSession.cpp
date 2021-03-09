#include <string>
#include <dlfcn.h>

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
        if (std::string(key) == "local.ip") return SetLocalIp(value);
	if (std::string(key) == "local.port") return SetLocalPort(value);
	if (std::string(key) == "peer.ip") return SetPeerIp(value);
	if (std::string(key) == "peer.port") return SetPeerPort(value);
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
	int result = pthread_create(&th, &attr, entry, 0);
	if (result != 0) PYCAI_ERROR("pthread_create fail, socket[%d], ret[%d]", skt_, result);
	ret = pthread_attr_destroy(&attr);
	if (ret != 0) PYCAI_ERROR("pthread_attr_destroy fail, socket[%d], ret[%d]", skt_, ret);
	return (result == 0); // equal to thread create success.
    }

private:
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

