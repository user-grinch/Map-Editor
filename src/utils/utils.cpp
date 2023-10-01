#include "pch.h"
#include "utils.h"
#include <CModelInfo.h>

std::string Utils::GetNameOfVehicleModel(int model) {
    CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(model);

    return (const char*)pInfo + 0x32;
}

// Clearly not stolen from MTA (sarcasm)
void Utils::CalcWorldCoors(CVector* vecScreen, CVector* vecWorld) {
    // Get the static view matrix as D3DXMATRIX
    D3DXMATRIXA16 m((float*)(0xB6FA2C));

    // Invert the view matrix
    D3DXMATRIXA16 minv;
    memset(&minv, 0, sizeof(D3DXMATRIXA16));
    m._44 = 1.0f;
    D3DXMatrixInverse(&minv, NULL, &m);

    DWORD* dwLenX = (DWORD*)(0xC17044);
    DWORD* dwLenY = (DWORD*)(0xC17048);

    // Reverse screen coordinates
    float fRecip = 1.0f / vecScreen->z;
    vecScreen->x /= fRecip * (*dwLenX);
    vecScreen->y /= fRecip * (*dwLenY);

    // Do an (inverse) transformation
    vecWorld->x = vecScreen->z * minv._31 + vecScreen->y * minv._21 + vecScreen->x * minv._11 + minv._41;
    vecWorld->y = vecScreen->z * minv._32 + vecScreen->y * minv._22 + vecScreen->x * minv._12 + minv._42;
    vecWorld->z = vecScreen->z * minv._33 + vecScreen->y * minv._23 + vecScreen->x * minv._13 + minv._43;
}

void Utils::CalcScreenCoords(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen) {
    D3DXMATRIX m((float*)(0xB6FA2C));

    DWORD* dwLenX = (DWORD*)(0xC17044);
    DWORD* dwLenY = (DWORD*)(0xC17048);

    vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
    vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
    vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

    double fRecip = (double)1.0 / vecScreen->z;
    vecScreen->x *= (float)(fRecip * (*dwLenX));
    vecScreen->y *= (float)(fRecip * (*dwLenY));
}

bool Utils::WorldToScreen(float x, float y, float z, float& screenX, float& screenY) {
    D3DXVECTOR3 vecScreen, vecWorld(x, y, z);

    CalcScreenCoords(&vecWorld, &vecScreen);

    screenX = vecScreen.x;
    screenY = vecScreen.y;

    if (vecScreen.z > 1.0f)
        return true;

    return false;
}

bool Utils::GetCursorPosition(CVector2D& vecCursor, CVector& vecWorld) {

    tagPOINT point;
    GetCursorPos(&point);

    tagPOINT windowPos = {0};
    ClientToScreen(RsGlobal.ps->window, &windowPos);

    CVector2D vecResolution = CVector2D(screen::GetScreenWidth(), screen::GetScreenHeight());
    point.x -= windowPos.x;
    point.y -= windowPos.y;
    if (point.x < 0)
        point.x = 0;
    else if (point.x > (long)vecResolution.x)
        point.x = (long)vecResolution.x;
    if (point.y < 0)
        point.y = 0;
    else if (point.y > (long)vecResolution.y)
        point.y = (long)vecResolution.y;

    vecCursor = CVector2D(((float)point.x) / vecResolution.x, ((float)point.y) / vecResolution.y);

    CVector vecScreen((float)((int)point.x), (float)((int)point.y), 300.0f);
    CalcWorldCoors(&vecScreen, &vecWorld);

    return true;
}


ImVec2 Utils::GetSize(short count, bool spacing) {

    if (count == 1) {
        spacing = false;
    }

    float factor = ImGui::GetStyle().ItemSpacing.x / 2.0f;
    float x;

    if (count == 3) {
        factor = ImGui::GetStyle().ItemSpacing.x / 1.403f;
    }

    if (spacing) {
        x = ImGui::GetWindowContentRegionWidth() / count - factor;
    } else {
        x = ImGui::GetWindowContentRegionWidth() / count;
    }

    return ImVec2(x, ImGui::GetFrameHeight() * 1.3f);
}

bool Utils::TraceEntity(CEntity*& pEntity, CVector& worldPos) {
    CVector2D cursor;
    CVector pos, cam;
    CColPoint outColPoint;
    cam = TheCamera.GetPosition();
    Utils::GetCursorPosition(cursor, pos);
    CWorld::ProcessLineOfSight(cam, pos, outColPoint, pEntity, 1, 0, 0, 1, 1, 0, 0, 0);

    if (pEntity == nullptr) {
        return false;
    }

    worldPos.x = outColPoint.m_vecPoint.x;
    worldPos.y = outColPoint.m_vecPoint.y;
    worldPos.z = outColPoint.m_vecPoint.z;

    return true;
}

void Utils::GetDegreeInRange(float *var) {
    while (*var < 0.0f) {
        *var += 360.0f;
    }

    while (*var > 360.0f) {
        *var -= 360.0f;
    }
}