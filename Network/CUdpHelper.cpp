#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "IUdpHelper.h"
#include "IPycaiLogger.h"

class CUdpHelper : public IUdpHelper
{
public:
    class CFactory : public IUdpHelper::IFactory
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
            return "CUdpHelper";
        }

        IUnknowObject* Create()
        {
            return new CUdpHelper();
        }
    };

    CUdpHelper() {}
    virtual ~CUdpHelper() {}

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
	if (skt_ >= 0) {
		close(skt_);
		PYCAI_INFO("stop udp socket[%d]", skt_);
		skt_ = -1;
	}
        delete this;
    }


    const char* GetLocalIp() const
    {
        return localIp_.c_str();
    }
    void SetLocalIp(const char* ip)
    {
        localIp_ = ip;
    }

    const char* GetPeerIp() const
    {
        return peerIp_.c_str();
    }
    void SetPeerIp(const char* ip)
    {
        peerIp_ = ip;
    }

    int GetLocalPort() const
    {
        return localPort_;
    }
    void SetLocalPort(int port)
    {
        localPort_ = port;
    }

    int GetPeerPort() const
    {
        return peerPort_;
    }
    void SetPeerPort(int port)
    {
        peerPort_ = port;
    }

    int Send(char* buf, int len)
    {
        if (len <= 0) {
            PYCAI_ERROR("input len is error");
            return -1;
        }

        if (!Init()) {
            PYCAI_ERROR("init fail.");
            return -1;
        }

        struct sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_port = htons(peerPort_);
        addr.sin_addr.s_addr = inet_addr(peerIp_.c_str());
        ssize_t ret = sendto(skt_, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
        int err = errno;
        if (ret <= 0) {
            PYCAI_ERROR("sendto fail, ret[%d], msg[%s]", (int)ret, strerror(err));
            return -1;
        }

        return (int)ret;
    }
    int Recv(char* buf, int len)
    {
        if (len <= 0) {
            PYCAI_ERROR("input len is error");
            return -1;
        }

        if (!Init()) {
            PYCAI_ERROR("init fail.");
            return -1;
        }

        struct sockaddr_in addr = { 0 };
        socklen_t sz = sizeof(addr);
        ssize_t ret = recvfrom(skt_, buf, len, 0, (struct sockaddr*)&addr, &sz);
        int err = errno;
        if (ret <= 0) {
            PYCAI_ERROR("recvfrom fail, ret[%d], msg[%s]", (int)ret, strerror(err));
            return -1;
        }

        std::string ip = inet_ntoa(addr.sin_addr);
        int port = ntohs(addr.sin_port);
        if (ip != peerIp_ || port != peerPort_) {
            PYCAI_WARN("recv not target peer, change to [%s:%d] from [%s:%d]", ip.c_str(), port, peerIp_.c_str(), peerPort_);
        }
        peerIp_ = ip;
	peerPort_ = port;
        return (int)ret;
    }

    bool Init() override
    {
        if (skt_ >= 0) return true;

        if (localIp_.empty() || localPort_ <= 0) {
            PYCAI_ERROR("local ip or port error.");
            return false;
        }

        int skt = socket(AF_INET, SOCK_DGRAM, 0);
        int err = errno;
        if (skt < 0) {
            PYCAI_ERROR("socket fail, ret[%d], msg[%s]", skt, strerror(err));
            return false;
        }

        struct sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_port = htons(localPort_);
        addr.sin_addr.s_addr = inet_addr(localIp_.c_str());
        int ret = bind(skt, (struct sockaddr*)&addr, sizeof(addr));
        err = errno;
        if (ret < 0) {
            PYCAI_ERROR("bind fail, ret[%d], msg[%s], ip[%s], port[%d]", ret, strerror(err), localIp_.c_str(), localPort_);
            close(skt);
            return false;
        }

        skt_ = skt;
        PYCAI_INFO("bind success, socket[%d], ip[%s], port[%d]", skt_, localIp_.c_str(), localPort_);
        return true;
    }

private:
    std::string localIp_;
    std::string peerIp_;
    int localPort_ = 0;
    int peerPort_ = 0;
    int skt_ = -1;
};

void CUdpHelperInit()
{
    static CUdpHelper::CFactory instance;
}

void CUdpHelperUninit()
{
}

