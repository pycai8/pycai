#include <string>

#include "IRtspHandler.h"
#include "IPycaiLogger.h"

class CRtspHandler : public IRtspHandler
{
public:
    class CFactory : public IRtspHandler::IFactory
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
            return "CRtspHandler";
        }

        IUnknowObject* Create()
        {
            return new CRtspHandler();
        }
    };

    CRtspHandler() {}
    virtual ~CRtspHandler() {}

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

    std::string Handle(const std::string& req) override
    {
        std:string reqType = GetRequestType(req);
        if (req == "OPTIONS") return HandleOptions(req);
        if (req == "DESCRIBE") return HandleOptions(req);
	if (req == "SETUP") return HandleSetup(req);
	if (req == "PLAY") return HandlePlay(req);
	if (req == "TEARDOWN") return HandleTeardown(req);
	PYCAI_ERROR("request[%s] not support.", reqType.c_str());
	return "";
    }

private:
    std::string localIp_;
    std::string peerIp_;
    int rtspLocalPort_ = 0;
    int rtspPeerPort_ = 0;
    int rtpLocalPort_ = 0;
    int rtpPeerPort_ = 0;
    int rtcpLocalPort_ = 0;
    int rtcpPeerPort_ = 0;
};

void CRtspHandlerInit()
{
    static CRtspHandler::CFactory instance;
}

void CRtspHandlerUninit()
{
}

