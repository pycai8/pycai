#ifndef PYCAI_ITCPSESSION_H_
#define PYCAI_ITCPSESSION_H_

#include "Component.h"

class ITcpSession : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "ITcpSession";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~ITcpSession() {}
    virtual bool Run(int skt) = 0;
};

#endif
