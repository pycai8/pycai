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
    
    loader->Load("libNetwork.so");
    loader->Load("libHttpMjpg.so");
    loader->Destroy();

    ITcpServer* server = CreateComponentObject<ITcpServer>("CTcpServer");
    if (!server) {
        PYCAI_ERROR("create CTcpServer fail.");
        return -1;
    }

    PYCAI_INFO("ready to start hls server ...");
    server->SetConfig("handler.class", "CHttpMjpgHandler");
    server->Listen("0.0.0.0", 8080);
    return 0;
}

