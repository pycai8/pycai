#include "ILibLoader.h"
#include "IPycaiLogger.h"

int main()
{
    ILibLoader* loader = CreateComponentObject<ILibLoader>("CDllLoader");
    if (!loader) {
        PYCAI_ERROR("create object of class[CDllLoader] failed.");
        return -1;
    }
    
    loader->Load("/d/pycai.dll");
    loader->Free("/d/pycai.dll");
    loader->Destroy();
    PYCAI_INFO("test component success.");
    return 0;
}

