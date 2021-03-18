#include "ITcpServer.h"
#include "ILibLoader.h"
#include "IPycaiLogger.h"

int main()
{
    ILibLoader* loader = CreateComponentObject<ILibLoader>("CSoLoader");
    if (!loader) {
        PYCAI_ERROR("create object of class[CSoLoader] failed.");
        return -1;
    }
    
    loader->Load("../Network/libNetwork.so");
    loader->Load("../Rtsp/libRtsp.so");
    loader->Destroy();

    ITcpServer* server = CreateComponentObject<ITcpServer>("CTcpServer");
    if (!server) {
        PYCAI_ERROR("create CTcpServer fail.");
        return -1;
    }

    PYCAI_INFO("ready to start rtsp server ...");
    server->SetConfig("handler.class", "CRtspHandler");
    server->Listen("0.0.0.0", 8554);
    return 0;
}

