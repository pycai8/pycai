#ifndef PYCAI_ITCPSERVER_H_
#define PYCAI_ITCPSERVER_H_

#include "Component.h"

class ITcpServer : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "ITcpServer";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~ITcpServer() {}
    virtual bool Listen(const char* ip, int port) = 0;
};

#endif
