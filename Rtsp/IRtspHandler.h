#ifndef PYCAI_IRTSPHANDLER_H_
#define PYCAI_IRTSPHANDLER_H_

#include "Component.h"

class IRtspHandler : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "IRtspHandler";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~IRtspHandler() {}
    virtual std::string Handle(const std::string& req) = 0;
};

#endif
