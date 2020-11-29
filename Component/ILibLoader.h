#ifndef PYCAI_ILIBLOADER_H_
#define PYCAI_ILIBLOADER_H_

#include "Component.h"

class ILibLoader : public IUnknowObject
{
public:
    class IFactory : public IUnknowFactory
    {
    public:
        virtual ~IFactory() {}
        static const char* StaticInterfaceId()
        {
            return "ILibLoader";
        }
        const char* GetInterfaceId() const override
        {
            return StaticInterfaceId();
        }
    };

    virtual ~ILibLoader() {}
    virtual bool Load(const char* file) = 0;
    virtual bool Free(const char* file) = 0;
};

#endif
