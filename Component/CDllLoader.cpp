#include "ILibLoader.h"
#include "IPycaiLogger.h"

class CDllLoader : public ILibLoader
{
public:
    class CFactory : public ILibLoader::IFactory
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
            return "CDllLoader";
        }

	IUnknowObject* Create()
        {
            return new CDllLoader();
        }
    };

    CDllLoader() {}
    virtual ~CDllLoader() {}

    const char* GetConfig(const char*) const override
    {
        return "";
    }

    bool SetConfig(const char*, const char*) override
    {
        return false;
    }

    void Destroy() override
    {
        delete this;
    }

    bool Load(const char* file) override
    {
        PYCAI_DEBUG("load dll[%s] success.", file);
        return true;
    }

    bool Free(const char* file) override
    {
        PYCAI_DEBUG("free dll[%s] success.", file);
        return true;
    }
};

void CDllLoaderInit()
{
    static CDllLoader::CFactory instance;
}

void CDllLoaderUninit()
{
}

