#include "IPycaiLogger.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <time.h>

using namespace std;

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

    void Logger(const char*, const char*,
                const char* file, int line,
                const char* func, int level,
                const char* fmt, ...) override
    {
        char str[1024] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf(str, sizeof(str) - 1, fmt, args);
	va_end(args);
        string now = GetNow();
	printf("%s", GetLevel(level));
	printf(" | %s | %s:%d %s ", now.c_str(), file, line, func);
	printf(" | %s\n", str);
    }

private:
    const char* GetLevel(int level)
    {
        switch (level) {
		case LEVEL_DEBUG: return "debug";
		case LEVEL_INFO: return " info";
		case LEVEL_WARN: return " warn ";
		case LEVEL_ERROR: return "error";
                case LEVEL_FATAL: return "fatal";
		default: return " none";
	}
    }

    string GetNow() const
    {
	    struct timeval tv;
	    struct tm ti;
	    gettimeofday(&tv, 0);
	    localtime_r(&(tv.tv_sec), &ti);
	    return to_string(ti.tm_year + 1900)
		    + (ti.tm_mon + 1 < 10 ? "/0" : "/") + to_string(ti.tm_mon + 1)
		    + (ti.tm_mday < 10 ? "/0" : "/") + to_string(ti.tm_mday)
		    + (ti.tm_hour < 10 ? " 0" : " ") + to_string(ti.tm_hour)
		    + (ti.tm_min < 10 ? ":0" : ":") + to_string(ti.tm_min) 
		    + (ti.tm_sec < 10 ? ":0" : ":") + to_string(ti.tm_sec)
		    + "." + GetMiliSec(tv.tv_usec / 1000);
    }

    string GetMiliSec(long time) const
    {
	    if (time < 10) return string("00") + to_string(time);
	    if (time < 100) return string("0") + to_string(time);
	    return to_string(time);
    }
};

void CPycaiLoggerInit()
{
    static CPycaiLogger::CFactory instance;
}

void CPycaiLoggerUninit()
{
}

