#ifndef PYCAI_IUDPHELPER_H_
#define PYCAI_IUDPHELPER_H_

#include "Component.h"

class IUdpHelper : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "IUdpHelper";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~IUdpHelper() {}

    virtual const char* GetLocalIp() const = 0;
    virtual void SetLocalIp(const char* ip) = 0;

    virtual const char* GetPeerIp() const = 0;
    virtual void SetPeerIp(const char* ip) = 0;

    virtual int GetLocalPort() const = 0;
    virtual void SetLocalPort(int port) = 0;

    virtual int GetPeerPort() const = 0;
    virtual void SetPeerPort(int port) = 0;
    
    virtual int Send(char* buf, int len) = 0;
    virtual int Recv(char* buf, int len) = 0;
};

#endif
