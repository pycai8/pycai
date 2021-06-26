#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ITcpServer.h"
#include "IPycaiLogger.h"
#include "ITcpSession.h"

class CTcpServer : public ITcpServer
{
public:
    class CFactory : public ITcpServer::IFactory
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
            return "CTcpServer";
        }

	IUnknowObject* Create()
        {
            return new CTcpServer();
        }
    };

    CTcpServer() {}
    virtual ~CTcpServer() {}

    const char* GetConfig(const char*) const override
    {
        return "";
    }

    bool SetConfig(const char* key, const char* value) override
    {
	if (std::string(key) == "handler.class") { hdrClass_ = value; return true; }
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool Listen(const char* ip, int port) override
    {
        int skt = socket(AF_INET, SOCK_STREAM, 0);
	int err = errno;
	if (skt < 0) {
            PYCAI_ERROR("socket fail, ret[%d], msg[%s]", skt, strerror(err));
	    return false;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	int ret = bind(skt, (struct sockaddr*)&addr, sizeof(addr));
	err = errno;
	if (ret != 0) {
            PYCAI_ERROR("bind fail, ip[%s], port[%d], ret[%d], msg[%s]", ip, port, ret, strerror(err));
	    close(skt);
	    return false;
	}

        int maxConnCount = 10;
	ret = listen(skt, maxConnCount);
	err = errno;
	if (ret != 0) {
            PYCAI_ERROR("listen fail, ip[%s], port[%d], ret[%d], msg[%s]", ip, port, ret, strerror(err));
	    close(skt);
	    return false;
	}

        PYCAI_INFO("listening %s:%d ...", ip, port);
	while (true) {
            usleep(10*1000); // sleep 10ms
            Accept(skt, ip, port);
	}

        close(skt);
	PYCAI_INFO("CTcpServer(%s:%d) stopped", ip, port);
	return true;
    }

private:
    void Accept(int skt, const char* ip, int port)
    {
        struct sockaddr_in tempAddr = { 0 };
        socklen_t tempLen = sizeof(tempAddr);
        int tempSkt = accept(skt, (struct sockaddr*)&tempAddr, &tempLen);
        int err = errno;
        if (tempSkt < 0) {
            PYCAI_ERROR("accept fail, ip[%s], port[%d], ret[%d], msg[%s]", ip, port, tempSkt, strerror(err));
            return;
        }

        std::string tempIp = inet_ntoa(tempAddr.sin_addr);
        int tempPort = ntohs(tempAddr.sin_port);
        ITcpSession* tcpSession = CreateComponentObject<ITcpSession>("CTcpSession");
        if (!tcpSession) {
            PYCAI_ERROR("Create CTcpSession(%s:%d <--> %s:%d) fail", ip, port, tempIp.c_str(), tempPort);
            close(tempSkt);
            return;
        }

        tcpSession->SetConfig("local.ip", ip);
        tcpSession->SetConfig("local.port", (char*)(unsigned long)port);
        tcpSession->SetConfig("peer.ip", tempIp.c_str());
        tcpSession->SetConfig("peer.port", (char*)(unsigned long)tempPort);
	tcpSession->SetConfig("handler.class", hdrClass_.c_str());
        if (!tcpSession->Run(tempSkt)) {
            PYCAI_ERROR("CTcpSession(%s:%d <--> %s:%d) run fail", ip, port, tempIp.c_str(), tempPort);
            tcpSession->Destroy();
            close(tempSkt);
            return;
        }

        PYCAI_INFO("CTcpSession(socket:%d, %s:%d <--> %s:%d) running ...", tempSkt, ip, port, tempIp.c_str(), tempPort);
    }

private:
    std::string hdrClass_;
};

void CTcpServerInit()
{
    static CTcpServer::CFactory instance;
}

void CTcpServerUninit()
{
}

