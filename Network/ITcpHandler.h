#ifndef PYCAI_ITCPHANDLER_H_
#define PYCAI_ITCPHANDLER_H_

#include "Component.h"

class ITcpHandler : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "ITcpHandler";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~ITcpHandler() {}
    virtual bool Handle(char* reqBuf, int& reqLen, char* respBuf, int& respLen) = 0;
};

#endif
