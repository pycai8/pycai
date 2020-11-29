#include "ILibLoader.h"
#include "IPycaiLogger.h"

class CSoLoader : public ILibLoader
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
            return "CSoLoader";
        }

	IUnknowObject* Create()
        {
            return new CSoLoader();
        }
    };

    CSoLoader() {}
    virtual ~CSoLoader() {}

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

void CSoLoaderInit()
{
    static CSoLoader::CFactory instance;
}

void CSoLoaderUninit()
{
}

