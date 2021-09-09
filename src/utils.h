#pragma once
#include "plugin.h"

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
};