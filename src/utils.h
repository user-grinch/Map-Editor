#pragma once
#include "plugin.h"
#include <d3dx9math.h>

class Utils
{
public:
    Utils() = delete;
    Utils(Utils&) = delete;

    static void CalcWorldCoors(CVector* vecScreen, CVector* vecWorld);
    static bool GetCursorPosition(CVector2D& vecCursor, CVector& vecWorld);
    static bool TraceEntity(CEntity*& pEntity, CVector& worldPos);
    static std::string GetNameOfVehicleModel(int model);
    static ImVec2 GetSize(short count = 1, bool spacing = true);
    static void GetDegreeInRange(float *var);
    static void CalcScreenCoords(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen);
    static bool WorldToScreen(float x, float y, float z, float& screenX, float& screenY);
};