#include <string>
#include <string.h>

#include "ITcpHandler.h"
#include "IPycaiLogger.h"
#include "ITsEncoder.h"

class CHlsHandler : public ITcpHandler
{
public:
    class CFactory : public ITcpHandler::IFactory
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
            return "CHlsHandler";
        }

        IUnknowObject* Create()
        {
            return new CHlsHandler();
        }
    };

    CHlsHandler() {}
    virtual ~CHlsHandler() {}

    const char* GetConfig(const char* key) const override
    {
        if (std::string(key) == "keep.alive") { return (keepAlive_ ? "true" : "false"); }
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
            localPort_ = (int)(unsigned long)value;
            return true;
        }
        if (std::string(key) == "peer.port") {
            peerPort_ = (int)(unsigned long)value;
            return true;
        }
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool Handle(char* reqBuf, int& reqLen, char* respBuf, int& respLen) override
    {
        std::string req(reqBuf, reqBuf + reqLen - 1);
        std::string reqType = GetRequestType(req);
	    keepAlive_ = GetKeepAlive(req);
        if (reqType == "m3u8") return HandleM3U8(respBuf, respLen);
        else if (reqType == "ts") return HandleTS(respBuf, respLen);
        else { PYCAI_ERROR("unknow request type[%s]", reqType.c_str()); return false; }
    }

private:
    bool GetKeepAlive(const std::string& req)
    {
        const char* tag= "Connection: close";
        return (req.find(tag) == std::string::npos);	
    }

    std::string GetRequestType(const std::string& req)
    {
        auto pos = req.find("\n");
        if (pos == std::string::npos) return "";

        auto reqLine = req.substr(0, pos);
        if (reqLine.find("playlist.m3u8") != std::string::npos) return "m3u8";
        else if (reqLine.find("playlist.ts") != std::string::npos) return "ts";
        else return "";
    }

    bool HandleM3U8(char* buf, int& len)
    {
        ITsEncoder* enc = CreateComponentObject<ITsEncoder>("CTsEncoder");
        if (enc == 0) {
            PYCAI_ERROR("can not create ts encoder");
        } else if (m_len <= 0) {
            m_len = sizeof(m_buf);
            if (!enc->H264ToTs("test.h264",m_buf, m_len)) {
                PYCAI_ERROR("convert 264 to ts fail.");
            }
            enc->Destroy();
        }
        

        std::string m3u8 = std::string("#EXTM3U\r\n") // must start with this
                            + "#EXT-X-VERSION:3\r\n" // use version 3
                            + "#EXT-X-TARGETDURATION:10\r\n" // max duration of ts is 5s
                            + "#EXT-X-MEDIA-SEQUENCE:" + std::to_string(staSeq_) + "\r\n" // start sequence of the m3u8
                            + "#EXTINF:" + std::to_string(duration_) + ",\r\n" // duration of the start ts clip
                            + std::to_string(staSeq_ + 0) + "playlist.ts\r\n" // the start ts clip
                            //+ "#EXT-X-DISCONTINUITY\r\n" // discontinuity exist
                            + "#EXTINF:" + std::to_string(duration_) + ",\r\n" // duration of the start ts clip
                            + std::to_string(staSeq_ + 1) + "playlist.ts\r\n" // the start ts clip
                            //+ "#EXT-X-DISCONTINUITY\r\n" // discontinuity exist
                            + "#EXTINF:" + std::to_string(duration_) + ",\r\n" // duration of the start ts clip
                            + std::to_string(staSeq_ + 2) + "playlist.ts\r\n"; // the start ts clip
        std::string resp = std::string("HTTP/1.1 200 OKi\r\n")
                            + "content-type: application/vnd.apple.mpegURL\r\n"
                            + "Access-Control-Allow-Origin: *\r\n"
                            + "Access-Control-Allow-Methods: GET, POST, PUT\r\n"
                            + "Content-Length: " + std::to_string(m3u8.size()) + "\r\n\r\n"
                            + m3u8;
        if (len <= resp.size()) {
            PYCAI_ERROR("buf size to small.");
            return false;
        }

        memcpy(buf, resp.c_str(), resp.size());
        len = resp.size();
        staSeq_ += 3;
        return true;
    }

    bool HandleTS(char* buf, int& len)
    {
        if (len < m_len + 2)
        {
            PYCAI_ERROR("input buf len is too small");
            return false;
        }
        memcpy(buf, m_buf, m_len);
        buf[m_len] = '\r';
        buf[m_len+1] = '\n';
        len = m_len + 2;
        return true;
    }

    std::string localIp_;
    std::string peerIp_;

    int localPort_ = 0;
    int peerPort_ = 0;

    static int staSeq_;
    double duration_ = 9.5;

    bool keepAlive_ = false;
    
    char m_buf[2 * 1024 * 1024] = { 0 };
    int m_len = 0;
};

int CHlsHandler::staSeq_ = 0;

void CHlsHandlerInit()
{
    static CHlsHandler::CFactory instance;
}

void CHlsHandlerUninit()
{
}

