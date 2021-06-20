#ifndef PYCAI_IMJPGENCODER_H_
#define PYCAI_IMJPGENCODER_H_

#include "Component.h"

class IMjpgEncoder : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "IMjpgEncoder";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~IMjpgEncoder() {}
    virtual bool H264ToMjpg(const char* h264File) = 0;
    virtual int GetJpgCount() const = 0;
    virtual int GetJpgLen(int index) const = 0;
    virtual const char* GetJpgData(int index) const = 0;
};

#endif
