#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ITcpHandler.h"
#include "IPycaiLogger.h"
#include "IMjpgEncoder.h"

class CHttpMjpgHandler : public ITcpHandler
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
            return "CHttpMjpgHandler";
        }

        IUnknowObject* Create()
        {
            return new CHttpMjpgHandler();
        }
    };

    CHttpMjpgHandler() {}
    virtual ~CHttpMjpgHandler() {}

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
        if (std::string(key) == "client.socket") {
            cliSkt_ = (int)(unsigned long)value;
        }
        return false;
    }

    void Destroy() override
    {
        m_running = false;
        sleep(5000);
        delete this;
    }

    bool Handle(char* reqBuf, int& reqLen, char* respBuf, int& respLen) override
    {
        std::string req(reqBuf, reqBuf + reqLen - 1);
        std::string reqType = GetRequestType(req);
	    keepAlive_ = GetKeepAlive(req);
        if (reqType != "mjpg") 
        { 
            PYCAI_ERROR("unknow request type[%s]", reqType.c_str()); 
            return false; 
        }

        std::string resp = "HTTP/1.0 200 OK\r\n";
        resp += "Access-Control-Allow-Origin:*\r\n";
        resp += "Connection: close\r\n";
        resp += "Server: MJPG-Streamer/0.2\r\n";
        resp += "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n";
        resp += "Pragma: no-cache\r\n";
        resp += "Expires: Mon, 3 Jan 2000 12 : 34 : 56 GMT\r\n";
        resp += "Content-Type: multipart/x-mixed-replace;boundary=boundarydonotcross\r\n\r\n";
        if (respLen <= resp.size())
        {
            PYCAI_ERROR("buffer to small");
            return false;
        }

        memcpy(respBuf, resp.c_str(), resp.size());
        respLen = resp.size();
        return StartMediaThread();
    }

private:

    bool GetKeepAlive(const std::string& req)
    {
        const char* tag = "Connection: close";
        return (req.find(tag) == std::string::npos);	
    }

    std::string GetRequestType(const std::string& req)
    {
        auto pos = req.find("\n");
        if (pos == std::string::npos) return "";

        auto reqLine = req.substr(0, pos);
        if (reqLine.find("playlist.mjpg") != std::string::npos) return "mjpg";
        else return "";
    }

    bool StartMediaThread()
    {
        pthread_attr_t attr;
        int ret = pthread_attr_init(&attr);
        if (ret != 0) PYCAI_ERROR("pthread_attr_init fail, socket[%d], ret[%d]", cliSkt_, ret);
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (ret != 0) PYCAI_ERROR("pthread_attr_setdetachstate fail, socket[%d], ret[%d]", cliSkt_, ret);

        pthread_t th;
        int result = pthread_create(&th, &attr, entry, this);
        if (result != 0) PYCAI_ERROR("pthread_create fail, socket[%d], ret[%d]", cliSkt_, result);
        ret = pthread_attr_destroy(&attr);
        if (ret != 0) PYCAI_ERROR("pthread_attr_destroy fail, socket[%d], ret[%d]", cliSkt_, ret);
        return (result == 0); // equal to thread create success.
    }

    static void* entry(void* arg)
    {
        CHttpMjpgHandler* hdr = (CHttpMjpgHandler*)arg;
        if (hdr == nullptr) {
            PYCAI_ERROR("arg is null.");
            return 0;
        }

        hdr->m_running = true;
        void* ret = hdr->Loop(arg);
        hdr->m_running = false;
        PYCAI_INFO("exit mjpeg translation thread.");
        return ret;
    }

    void* Loop(void* arg)
    {
        IMjpgEncoder* enc = CreateComponentObject<IMjpgEncoder>("CMjpgEncoder");
        if (!enc)
        {
            PYCAI_ERROR("can not create mjpg encoder");
            return 0;
        }

        if (!enc->H264ToMjpg("test.h264"))
        {
            PYCAI_ERROR("transcode 264 to mjpg fail.");
            enc->Destroy();
            return 0;
        }

        PYCAI_INFO("transcode success, jpeg count[%d]", enc->GetJpgCount());

        for (int i = 0; m_running && i < enc->GetJpgCount(); i++)
        {
            sleep(40);//25ms
            SendOneJpg((char*)enc->GetJpgData(i), enc->GetJpgLen(i));
        }

        enc->Destroy();
        return 0;
    }

    bool SendOneJpg(char* buf, int len)
    {
        std::string head = "\r\n--boundarydonotcross\r\n";
        head += "Content-Type: image/jpeg\r\n";
        head += "Content-Length: " + std::to_string(len) + "\r\n";
        head += "X-Timestamp: 0.000000\r\n\r\n";
        SendBuffer((char*)head.c_str(), head.size());
        SendBuffer(buf, len);
        return true;
    }

    bool SendBuffer(char* buf, int len)
    {
        if (cliSkt_ < 0)
        {
            PYCAI_ERROR("client socket is invalidate.");
            return false;
        }

        ssize_t ret = send(cliSkt_, buf, len, 0);
        int err = errno;
        if (ret < 0)
        {
            PYCAI_ERROR("send data fail, socket[%d], return[%d], error[%s].", cliSkt_, ret, strerror(err));
            return false;
        }
        else
        {
            PYCAI_DEBUG("send data success, socket[%d], return[%d], error[%s].", cliSkt_, ret, strerror(err));
        }

        return true;
    }

    std::string localIp_;
    std::string peerIp_;

    int localPort_ = 0;
    int peerPort_ = 0;
    int cliSkt_ = -1;

    bool keepAlive_ = false;
    bool m_running = false;
};

void CHttpMjpgHandlerInit()
{
    static CHttpMjpgHandler::CFactory instance;
}

void CHttpMjpgHandlerUninit()
{
}
