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

    bool SetConfig(const char* key, const char* value) override
    {
		if (std::string(key) == "local.ip") {
			localIp_ = value;
			return true;
		}
		if (std::string(key) == "peer.ip") {
			peerIp_ = value;
			return true;
		}
		if (std::string(key) == "local.port") {
			rtspLocalPort_ = (int)(unsigned long)value;
			return true;
		}
		if (std::string(key) == "peer.port") {
			rtspPeerPort_ = (int)(unsigned long)value;
			return true;
		}
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
	std::string GetRequestType(const std::string& req)
	{
		auto pos = req.find(" rtsp://");
		if (pos == std::string::npos) {
			return "";
		}

		return req.substr(0, pos);
	}

    int GetRequestSequence(const std::string& req)
	{
		auto pos = req.find("CSeq: ");
		if (pos == std::string::npos) {
			return 0;
		}
		int ret = 0;
		for (auto i = pos; i < req.size() && req[i] != '\r' && req[i] != '\n'; ++i) {
			if (!('0'<= req[i] && req[i] <= '9')) continue;
			ret = ret * 10 + req[i] - '0';
		}
		return ret;
	}

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

