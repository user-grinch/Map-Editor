#pragma once
#include <string>

#include <CVector.h>
#include <CQuaternion.h>
#include <CObject.h>
#include <CPools.h>

/*
  EntityInfo MapEditor class
  Stores MapEditor object specific data
*/
class EntityInfo {
  private:
    CVector m_Euler {0, 0, 0};
    CQuaternion m_Quat {{0, 0, 0}, 0};

  public:
    int m_nHandle; 
    CObject *m_pObj; 
    std::string m_sModelName;

    EntityInfo(CObject *obj);
    CVector GetEuler();
    void SetEuler(CVector rot);
    CQuaternion GetQuat();
    void SetQuat(CQuaternion quat);
};