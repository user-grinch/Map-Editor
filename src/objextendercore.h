#pragma once
#include "CPools.h"
#include "Extender.h"
#include "Events.h"
#include <vector>

class CObject;

class ObjExtendersHandler : public plugin::ExtendersHandler<CObject> {
public:
    static void Add(plugin::ExtenderInterface<CObject> *extender) {
        static plugin::ThiscallEvent <plugin::AddressList<0x5A1D4C, plugin::H_CALL,
            0x5A1DC8, plugin::H_CALL, 0x5A1E72, plugin::H_CALL,
            0x5A1FC3, plugin::H_CALL,
            0x5A2038, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickN<CObject*, 0>, 
            void(CObject*)>  objectInitEvent;
        static_data& data = get_data();
        data.extenders.push_back(extender);
        if (!data.injected) {
            plugin::Events::initPoolsEvent.after += Allocate;
            objectInitEvent.after += Constructor;
            plugin::Events::objectDtorEvent.after += Destructor;
            data.injected = true;
        }
    }
};

template <typename T> class ObjExtender : public ExtenderInterface<CObject> {
    T **blocks;
    unsigned int numBlocks;

    void AllocateBlocks() {
        numBlocks = CPools::ms_pObjectPool->m_nSize;
        blocks = new T*[numBlocks];
        for (unsigned int i = 0; i < numBlocks; i++)
            blocks[i] = 0;
    }

    void OnConstructor(CObject *object) {
        blocks[CPools::ms_pObjectPool->GetIndex(object)] = new T(object);
    }

    void OnDestructor(CObject *object) {
        delete blocks[CPools::ms_pObjectPool->GetIndex(object)];
        blocks[CPools::ms_pObjectPool->GetIndex(object)] = 0;
    }
public:
    ObjExtender() {
        blocks = 0;
        ObjExtendersHandler::Add(this);
    }

    ~ObjExtender() {
        for (unsigned int i = 0; i < numBlocks; i++)
            delete blocks[i];
        delete[] blocks;
    }

    T &Get(CObject *object) {
        return *blocks[CPools::ms_pObjectPool->GetIndex(object)];
    }
};
