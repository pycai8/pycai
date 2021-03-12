#include <string>
#include <dlfcn.h>

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
        auto ret = dlopen(file, RTLD_NOW | RTLD_LOCAL);
	if (ret == 0) {
	        std::string err = dlerror();
		PYCAI_ERROR("dlopen(%s) fail, %s", file, err.c_str());
		return false;
	}
        PYCAI_DEBUG("dlopen(%s) success.", file);
        return true;
    }

    bool Free(const char* file) override
    {
        PYCAI_ERROR("free dll[%s] fail, not implementation.", file);
        return false;
    }
};

void CSoLoaderInit()
{
    static CSoLoader::CFactory instance;
}

void CSoLoaderUninit()
{
}

