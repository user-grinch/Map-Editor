#include "pch.h"
#include "entityinfo.h"
#include <CQuaternion.h>
#include "utils/utils.h"
#include <RenderWare.h>

EntityInfo::EntityInfo(CObject *pObj) {
    if (!pObj) {
        return;
    }

    m_pObj = pObj;
    m_nHandle = CPools::GetObjectRef(pObj);
}


RwV3d axyz = {1, 1, 1};

void EntityInfo::SetEuler(CVector rot) {
    CQuaternion q = {{0, 0, 0}, 1};

    RwV3d ax = {1, 0, 0};
    RwV3d ay = {0, 1, 0};
    RwV3d az = {0, 0, 1};
    RtQuatRotate((RtQuat*)&q, &ax, rot.x, rwCOMBINEREPLACE);
    q.Normalise();
    RtQuatRotate((RtQuat*)&q, &ay, rot.y, rwCOMBINEPOSTCONCAT);
    q.Normalise();
    RtQuatRotate((RtQuat*)&q, &az, rot.z, rwCOMBINEPOSTCONCAT);
    q.Normalise();
    SetQuat(q);
    m_Euler = rot;
}

CVector EntityInfo::GetEuler() {
    CQuaternion q = GetQuat();

    // Normalize quaternion
    float len = sqrt(q.imag.x * q.imag.x + q.imag.y * q.imag.y + q.imag.z * q.imag.z + q.real * q.real);
    float x = q.imag.x / len;
    float y = q.imag.y / len;
    float z = q.imag.z / len;
    float w = q.real / len;

    // Convert to rotation matrix
    float m00 = 1 - 2 * (y * y + z * z);
    float m01 = 2 * (x * y - z * w);
    float m02 = 2 * (x * z + y * w);
    float m10 = 2 * (x * y + z * w);
    float m11 = 1 - 2 * (x * x + z * z);
    float m12 = 2 * (y * z - x * w);
    float m20 = 2 * (x * z - y * w);
    float m21 = 2 * (y * z + x * w);
    float m22 = 1 - 2 * (x * x + y * y);

    // Static fields to hold last known roll/yaw
    static float lastRoll = 0.0f;
    static float lastYaw = 0.0f;

    float pitch, roll, yaw;

    // Gimbal lock detection
    if (fabs(m20) >= 0.9999f) {
        pitch = asin(-m20);
        roll = lastRoll;
        yaw = lastYaw;
    } else {
        pitch = asin(-m20);
        roll = atan2(m21, m22);
        yaw = atan2(m10, m00);

        // Update static fields
        lastRoll = roll;
        lastYaw = yaw;
    }

    // Convert radians to degrees
    roll = DEG_TO_RAD(roll);
    pitch = DEG_TO_RAD(pitch);
    yaw = DEG_TO_RAD(yaw);

    return CVector(roll, pitch, yaw);
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

    // //void __thiscall CMatrix::ConvertToEulerAngles(CMatrix *this, float *pX, float *pY, float *pZ, unsigned int flags)
    // CallMethod<0x59A840, int>((int)m_pObj->GetMatrix(), &m_Euler.x, &m_Euler.y, &m_Euler.z, 0); 
    // m_Euler.x = Utils::NormalizeAngle(RAD_TO_DEG(m_Euler.x));
    // m_Euler.y = Utils::NormalizeAngle(RAD_TO_DEG(m_Euler.y));
    // m_Euler.z = Utils::NormalizeAngle(RAD_TO_DEG(m_pObj->GetHeading())) - 90.0f;
}