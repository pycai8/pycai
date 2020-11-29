#include "Component.h"
#include "IPycaiLogger.h"

#include <map>
#include <string>

using namespace std;

static map<string, map<string, IUnknowFactory*>> g_factories;

void IUnknowFactory::RegisterFactory()
{
    string iid = GetInterfaceId();
    if (iid.empty()) {
        PYCAI_ERROR("The interface ID is empty.");
        return;
    }

    string cid = GetClassId();
    if (cid.empty()) {
        PYCAI_ERROR("The class ID of interface[%s] is empty.", iid.c_str());
        return;
    }

    auto iit = g_factories.find(iid);
    if (iit == g_factories.end()) {
        map<string, IUnknowFactory*> factories;
        factories[cid] = this;
        g_factories[iid] = factories;
        PYCAI_DEBUG("First register class[%s] of interface[%s] success.", cid.c_str(), iid.c_str());
        return;
    }

    auto cit = iit->second.find(cid);
    if (cit == iit->second.end()) {
        iit->second[cid] = this;
        PYCAI_DEBUG("Register class[%s] of interface[%s] success.", cid.c_str(), iid.c_str());
        return;
    }
    
    PYCAI_ERROR("The class[%s] of interface[%s] had allready registered.", cid.c_str(), iid.c_str());
}

void IUnknowFactory::UnregisterFactory()
{
    string iid = GetInterfaceId();
    if (iid.empty()) {
        PYCAI_ERROR("The interface ID is empty.");
        return;
    }

    string cid = GetClassId();
    if (cid.empty()) {
        PYCAI_ERROR("The class ID of interface[%s] is empty.", iid.c_str());
        return;
    }

    auto iit = g_factories.find(iid);
    if (iit == g_factories.end()) {
        PYCAI_ERROR("The class[%s] and interface[%s] not registered.", cid.c_str(), iid.c_str());
        return;
    }

    auto cit = iit->second.find(cid);
    if (cit == iit->second.end()) {
        PYCAI_ERROR("The class[%s] of interface[%s] not registered.", cid.c_str(), iid.c_str());
        return;
    }

    iit->second.erase(cid);
    if (iit->second.empty()) {
        g_factories.erase(iit);
    }
    PYCAI_DEBUG("The class[%s] of interface[%s] unregister success.", cid.c_str(), iid.c_str());
}

IUnknowObject* CreateComponentObject(const char* iid, const char* cid)
{
    if (!iid || !(*iid)) {
        if (std::string("IPycaiLogger") != iid) PYCAI_ERROR("The interface ID is empty.");
        return 0;
    }

    if (!cid || !(*cid)) {
        if (std::string("IPycaiLogger") != iid) PYCAI_ERROR("The class ID of interface[%s] is empty.", iid);
        return 0;
    }

    auto iit = g_factories.find(iid);
    if (iit == g_factories.end()) {
        if (std::string("IPycaiLogger") != iid) PYCAI_ERROR("The class[%s] and interface[%s] not registered.", cid, iid);
        return 0;
    }

    auto cit = iit->second.find(cid);
    if (cit == iit->second.end()) {
        if (std::string("IPycaiLogger") != iid) PYCAI_ERROR("The class[%s] of interface[%s] not registered.", cid, iid);
        return 0;
    }

    IUnknowObject* obj = cit->second->Create();
    if (!obj) {
        if (std::string("IPycaiLogger") != iid) PYCAI_ERROR("Create object of class[%s] of interface[%s] failed.", cid, iid);
    } else {
        if (std::string("IPycaiLogger") != iid) PYCAI_DEBUG("Create object of class[%s] of interface[%s] success.", cid, iid);
    }
    return obj;
}

