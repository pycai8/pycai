#include <string>
#include <string.h>

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

        HTTP/1.0 200 OK
        Access-Control-Allow-Origin: *
        Connection: close
        Server: MJPG-Streamer/0.2
        Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0
        Pragma: no-cache
        Expires: Mon, 3 Jan 2000 12:34:56 GMT
        Content-Type: multipart/x-mixed-replace;boundary=boundarydonotcross        
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

    bool HandleMjpg(char* buf, int& len)
    {
        StartMediaThread();
    }

    IMjpgEncoder* TransCode(const char* h264File)
    {
        IMjpgEncoder* ret = CreateComponentObject<IMjpgEncoder>("CMjpgEncoder");
        if (!ret)
        {
            PYCAI_ERROR("can not create mjpg encoder");
            return 0;
        }

        if (!ret->H264ToMjpg(h264File))
        {
            PYCAI_ERROR("transcode 264 to mjpg fail.");
            ret->Destroy();
            return 0;
        }

        return ret;
    }

    bool StartMediaThread()
    {
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

    static void* entry(void* arg)
    {
        if (arg == nullptr) {
            PYCAI_ERROR("arg is null.");
            return 0;
        }
        CHttpMjpgHandler* s = (CHttpMjpgHandler*)arg;
        s->Loop();
        return 0;
    }
 
    void Loop()
    {
        IMjpgEncoder* enc = TransCode("test.h264");
        for (int i = 0; m_running && i < enc->GetJpgCount(); i++)
        {
            sleep(40);//25ms
            SendOneJpg(enc->GetJpgData(i), enc->GetJpgLen(i));
        }
        enc->Destroy();
        m_running = false;
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

