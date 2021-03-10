#ifndef PYCAI_IMEDIASESSION_H_
#define PYCAI_IMEDIASESSION_H_

#include "Component.h"
#include "IUdpHelper.h"

class IMediaSession : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "IMediaSession";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~IMediaSession() {}
    virtual void Init(IUdpHelper* value) = 0;
    virtual bool Run(const char* file) = 0;
};

#endif
