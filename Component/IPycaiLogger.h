#ifndef PYCAI_IPYCAILOGGER_H_
#define PYCAI_IPYCAILOGGER_H_

#include "Component.h"


enum { LEVEL_DEBUG = 0, LEVEL_INFO, LEVEL_WARN, LEVEL_ERROR, LEVEL_FATAL };

class IPycaiLogger : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "IPycaiLogger";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~IPycaiLogger() {}
    virtual void Logger(const char* date, const char* time,
                        const char* file, int line,
                        const char* func, int level,
                        const char* fmt, ...) = 0;
};

#define PYCAI_LOGGER(level, fmt, ...) \
{ \
    IPycaiLogger* log = CreateComponentObject<IPycaiLogger>("CPycaiLogger"); \
    if (log) { \
        log->Logger(__DATE__, __TIME__, __FILE__, __LINE__, __FUNCTION__, level, fmt, ##__VA_ARGS__); \
    } \
} \


#define PYCAI_DEBUG(fmt, ...) PYCAI_LOGGER(LEVEL_DEBUG, fmt, ##__VA_ARGS__);
#define PYCAI_INFO(fmt, ...) PYCAI_LOGGER(LEVEL_INFO, fmt, ##__VA_ARGS__);
#define PYCAI_WARN(fmt, ...) PYCAI_LOGGER(LEVEL_WARN, fmt, ##__VA_ARGS__);
#define PYCAI_ERROR(fmt, ...) PYCAI_LOGGER(LEVEL_ERROR, fmt, ##__VA_ARGS__);
#define PYCAI_FATAL(fmt, ...) PYCAI_LOGGER(LEVEL_FATAL, fmt, ##__VA_ARGS__);

#endif
