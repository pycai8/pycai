#include "IPycaiLogger.h"

#include <iostream>

class CPycaiLogger : public IPycaiLogger
{
public:
    class CFactory : public IPycaiLogger::IFactory
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
            return "CPycaiLogger";
        }

	IUnknowObject* Create()
        {
            static CPycaiLogger instance;
	    return &instance;
        }
    };

    CPycaiLogger() {}
    virtual ~CPycaiLogger() {}

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

    void Logger(const char* date, const char* time,
                const char* file, int line,
                const char* func, int level,
                const char* fmt, ...) override
    {
        std::cout << date << time << file << line << func << level << fmt << std::endl;
    }
};

void CPycaiLoggerInit()
{
    static CPycaiLogger::CFactory instance;
}

void CPycaiLoggerUninit()
{
}

