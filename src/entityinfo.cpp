#include "pch.h"
#include "entityinfo.h"
#include <CQuaternion.h>
#include "utils/utils.h"

EntityInfo::EntityInfo(CObject *pObj) {
    if (!pObj) {
        return;
    }

    m_pObj = pObj;
    m_nHandle = CPools::GetObjectRef(pObj);

    pObj->GetOrientation(m_Euler.x, m_Euler.y, m_Euler.z);
    m_Euler.x = DEG_TO_RAD(m_Euler.x);
    m_Euler.y = DEG_TO_RAD(m_Euler.y);
    m_Euler.z = DEG_TO_RAD(m_pObj->GetHeading());
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
    CQuaternion quat;
    plugin::Command<plugin::Commands::GET_OBJECT_QUATERNION>(m_nHandle, &quat.imag.x, &quat.imag.y, 
         &quat.imag.z, &quat.real);
    return quat;
}

void EntityInfo::SetQuat(CQuaternion quat) {
    CWorld::Remove(m_pObj);
    plugin::Command<plugin::Commands::SET_OBJECT_QUATERNION>(m_nHandle, quat.imag.x, quat.imag.y, 
         quat.imag.z, quat.real);

    m_pObj->UpdateRwMatrix();
    m_pObj->UpdateRwFrame();
    CWorld::Add(m_pObj);

    //void __thiscall CMatrix::ConvertToEulerAngles(CMatrix *this, float *pX, float *pY, float *pZ, unsigned int flags)
    CallMethod<0x59A840, int>((int)m_pObj->GetMatrix(), &m_Euler.x, &m_Euler.y, &m_Euler.z, 0); 
    m_Euler.x = Utils::NormalizeAngle(RAD_TO_DEG(m_Euler.x));
    m_Euler.y = Utils::NormalizeAngle(RAD_TO_DEG(m_Euler.y));
    m_Euler.z = Utils::NormalizeAngle(RAD_TO_DEG(m_pObj->GetHeading())) - 90.0f;
}