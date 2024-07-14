#pragma once
#include "pch.h"
#include "entityinfo.h"
#include "objextendercore.h"

/*
* Entity Manager Class
* Handles MapEditor entities
*/
class EntityMgr {
  public:
    using IdeName = std::string;
    using ModelID = unsigned int;
    using ModelName = std::string;

    std::map<IdeName, std::map<ModelID, ModelName>> m_NameList;
    std::vector<IdeName> m_IdeList;
    std::vector<CObject*> m_pPlaced; 
    ObjExtender<EntityInfo> m_Info;
    CObject* m_pSelected; 
    size_t m_nTotalEntities; 

    struct {
      int m_nModel = -1;
      CVector m_Rot;
    } ClipBoard;

    EntityMgr();
    EntityMgr(EntityMgr&);
    std::string FindNameFromModel(int model);
    float GetBoundingBoxGroundZ(CObject *pObj);
};

extern EntityMgr EntMgr;