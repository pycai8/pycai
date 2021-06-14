#ifndef PYCAI_ITSENCODER_H_
#define PYCAI_ITSENCODER_H_

#include "Component.h"

class ITsEncoder : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "ITsEncoder";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~ITsEncoder() {}
    virtual bool H264ToTs(const char* h264File, char* data, int& len) = 0;
};

#endif
