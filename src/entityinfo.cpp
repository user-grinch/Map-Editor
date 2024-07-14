#include "pch.h"
#include "entityinfo.h"
#include <CQuaternion.h>

EntityInfo::EntityInfo(CObject *pObj) {
    if (!pObj) {
        return;
    }

    m_pObj = pObj;
    m_nHandle = CPools::GetObjectRef(pObj);

    pObj->GetOrientation(m_Euler.x, m_Euler.y, m_Euler.z);
    m_Euler.x = DEG_TO_RAD(m_Euler.x);
    m_Euler.y = DEG_TO_RAD(m_Euler.y);
    m_Euler.z = DEG_TO_RAD(m_Euler.z);

    if (m_pObj->m_pRwObject) {
        m_Quat.Set(*RwFrameGetMatrix(RwFrameGetParent(m_pObj->m_pRwObject)));
    }
}

void EntityInfo::SetEuler(CVector rot) {
    m_Euler = rot;
    CWorld::Remove(m_pObj);
    m_pObj->SetOrientation(DEG_TO_RAD(rot.x), DEG_TO_RAD(rot.y), DEG_TO_RAD(rot.z));
    m_pObj->UpdateRwMatrix();
    m_pObj->UpdateRwFrame();
    CWorld::Add(m_pObj);
}

CVector EntityInfo::GetEuler() {
    return m_Euler;
}

CQuaternion EntityInfo::GetQuat() {
    plugin::Command<plugin::Commands::GET_OBJECT_QUATERNION>(m_nHandle, &m_Quat.imag.x, &m_Quat.imag.y, 
         &m_Quat.imag.z, &m_Quat.real);
    return m_Quat;
}

void EntityInfo::SetQuat(CQuaternion quat) {
    m_Quat = quat;
    plugin::Command<plugin::Commands::SET_OBJECT_QUATERNION>(m_nHandle, m_Quat.imag.x, m_Quat.imag.y, 
         m_Quat.imag.z, m_Quat.real);
}