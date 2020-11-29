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
    loader->Free("../Network/libNetwork.so");
    loader->Destroy();
    PYCAI_INFO("test component success.");
    return 0;
}

