#include <string>

#include "IMediaSession.h"
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
        std::string reqType = GetRequestType(req);
        int seq = GetRequestSequence(req);
        if (reqType == "OPTIONS") return HandleOptions(req, seq);
        if (reqType == "DESCRIBE") return HandleDescribe(req, seq);
        if (reqType == "SETUP") return HandleSetup(req, seq);
        if (reqType == "PLAY") return HandlePlay(req, seq);
        if (reqType == "TEARDOWN") return HandleTeardown(req, seq);
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

    void ParseClientPorts(const std::string& req)
    {
        auto pos = req.find("client_port=");
        if (pos == std::string::npos) return;

        pos += 12; // length of "client_port="
        rtpPeerPort_ = 0;
        rtcpPeerPort_ = 0;
        bool flag = false;
        for (int i = pos; i < req.size() && req[i] != ';' && req[i] != '\r' && req[i] != '\n'; ++i) {
            if (req[i] == '-') flag = true;
            if (!('0' <= req[i] && req[i] <= '9')) continue;
            if (flag) rtcpPeerPort_ = rtcpPeerPort_ * 10 + (req[i] - '0');
            else rtpPeerPort_ = rtpPeerPort_ * 10 + (req[i] - '0');
        }
	
	PYCAI_INFO("parse client port, rtcpPeerPort=%d, rtpPeerPort=%d", rtcpPeerPort_, rtpPeerPort_);
    }

    std::string HandleOptions(const std::string& req, int seq)
    {
        return std::string("RTSP/1.0 200 OK\r\n")
                + "CSeq: " + std::to_string(seq) + "\r\n"
                + "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n";
    }

    std::string HandleDescribe(const std::string& req, int seq)
    {
        std::string des = std::string("v=0\r\n") // sdp version
                        + "o=- " + GetSessionId() + " 1 IN IP4 " + localIp_ + "\r\n" // <username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
                        + "t=0 0\r\n" // 时间信息，分别表示开始时间和结束时间
                        + "a=control:*\r\n"
                        + "a=type:broadcast\r\n"
                        + "m=video 0 RTP/AVP 96\r\n" // video通过RTP传送，其payload值为96, 传送的端口0(还没定)
                        + "c=IN IP4 " + localIp_ + "\r\n" // 网络协议，地址的类型，连接地址
                        + "a=rtpmap:96 H264/90000\r\n" // rtpmap的信息，表示video为H264, 其ClockRate为90000
                        + "a=framerate:25\r\n" // 视频帧率25
                        + "a=control:track0\r\n";
        return std::string("RTSP/1.0 200 OK\r\n")
                + "CSeq: " + std::to_string(seq) + "\r\n"
                + "Content-Length: " + std::to_string(des.size()) + "\r\n"
                + "Content-Type: application/sdp\r\n\r\n"
                + des;
    }

    std::string HandleSetup(const std::string& req, int seq)
    {
        ParseClientPorts(req);
        GenServerPorts();
        CreateSessions();
        return std::string("RTSP/1.0 200 OK\r\n")
                + "CSeq: " + std::to_string(seq) + "\r\n"
                + "Transport: RTP/AVP;unicast;client_port=" 
                    + std::to_string(rtpPeerPort_) + "-" + std::to_string(rtcpPeerPort_) 
                    + ";server_port=" 
                    + std::to_string(rtpLocalPort_)+"-" + std::to_string(rtcpLocalPort_) + "\r\n"
                + "Session: " + GetSessionId() + "\r\n\r\n";
    }

    std::string HandlePlay(const std::string& req, int seq)
    {
        StartSessions();
        return std::string("RTSP/1.0 200 OK\r\n")
                + "CSeq: " + std::to_string(seq) + "\r\n"
                + "Range: npt=0.000-\r\n"
                + "Session: " + GetSessionId() + "; timeout=60\r\n\r\n";
    }

    std::string HandleTeardown(const std::string& req, int seq)
    {
        StopSessions();
        return std::string("RTSP/1.0 200 OK\r\nCSeq: ") + std::to_string(seq) + "\r\n\r\n";
    }

    std::string GetSessionId() const
    {
        return std::to_string((unsigned long)this);
    }

    void GenServerPorts()
    {
        int port = rand() % 100000; // port from 10000 to 99999
        if (port % 2 != 0) port -= 1;
        if (port < 10000) port += 10000;
        rtpLocalPort_ = port;
        rtcpLocalPort_ = port + 1;
    }

    void CreateSessions()
    {
        IUdpHelper* rtpHelper = CreateComponentObject<IUdpHelper>("CUdpHelper");
        rtpHelper->SetLocalIp(localIp_.c_str());
        rtpHelper->SetLocalPort(rtpLocalPort_);
        rtpHelper->SetPeerIp(peerIp_.c_str());
        rtpHelper->SetPeerPort(rtpPeerPort_);
        rtp_ = CreateComponentObject<IMediaSession>("CRtpSession");
        rtp_->Init(rtpHelper);

        IUdpHelper* rtcpHelper = CreateComponentObject<IUdpHelper>("CUdpHelper");
        rtcpHelper->SetLocalIp(localIp_.c_str());
        rtcpHelper->SetLocalPort(rtcpLocalPort_);
        rtcpHelper->SetPeerIp(peerIp_.c_str());
        rtcpHelper->SetPeerPort(rtcpPeerPort_);
        rtcp_ = CreateComponentObject<IMediaSession>("CRtcpSession");
        rtcp_->Init(rtcpHelper);
    }

    void StartSessions()
    {
        rtp_->Start("test.h264");
        rtcp_->Start("test.h264");
    }

    void StopSessions()
    {
        rtp_->Stop();
        rtcp_->Stop();
    }

    std::string localIp_;
    std::string peerIp_;

    int rtspLocalPort_ = 0;
    int rtspPeerPort_ = 0;
    int rtpLocalPort_ = 0;
    int rtpPeerPort_ = 0;
    int rtcpLocalPort_ = 0;
    int rtcpPeerPort_ = 0;

    IMediaSession* rtp_ = 0;
    IMediaSession* rtcp_ = 0;
};

void CRtspHandlerInit()
{
    static CRtspHandler::CFactory instance;
}

void CRtspHandlerUninit()
{
}

