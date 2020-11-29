#ifndef PYCAI_COMPONENT_H_
#define PYCAI_COMPONENT_H_

class IUnknowObject
{
public:
    virtual ~IUnknowObject() {}
    virtual const char* GetConfig(const char* key) const = 0;
    virtual bool SetConfig(const char* key, const char* value) = 0;
    virtual void Destroy() = 0;
};

class IUnknowFactory
{
public:
    virtual ~IUnknowFactory() {}
    void RegisterFactory();
    void UnregisterFactory();
    virtual const char* GetInterfaceId() const = 0;
    virtual const char* GetClassId() const = 0;
    virtual IUnknowObject* Create() = 0;
};

IUnknowObject* CreateComponentObject(const char* iid, const char* cid);

template <typename T>
T* CreateComponentObject(const char* cid)
{
    return (T*)CreateComponentObject(T::IFactory::StaticInterfaceId(), cid);
}

#endif

