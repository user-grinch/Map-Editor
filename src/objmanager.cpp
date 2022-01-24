#include "pch.h"
#include "editor.h"
#include "objmanager.h"
#include <CRenderer.h>
#include <CModelInfo.h>

/*
*  Part of the source is taken from DrawColsSA by Sergeanur
*  https://github.com/Sergeanur/
*/

static void RenderLineWithClipping(float x1, float y1, float z1, float x2, float y2, float z2, unsigned int c1, unsigned int c2)
{
    ((void (__cdecl *)(float, float, float, float, float, float, unsigned int, unsigned int))0x6FF4F0)(x1, y1, z1, x2, y2, z2, c1, c2);
}

float ObjManager::GetBoundingBoxGroundZ(CObject *pObj)
{
    if (!pObj)
    {
        return 0.0f;
    }
    CMatrix *matrix = pObj->GetMatrix();
    CColModel *pColModel = pObj->GetColModel();
    CVector min = pColModel->m_boundBox.m_vecMin;
    CVector max = pColModel->m_boundBox.m_vecMax;

    CVector workVec = min;
    workVec.x = max.x;
    workVec.y = max.y;
    CVector ground = *matrix * workVec;

    return ground.z;
}

void ObjManager::DrawColModel(const CMatrix& matrix, const CColModel& colModel)
{
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

    if (m_bDrawBoundingBox)
    {
        CVector min = colModel.m_boundBox.m_vecMin;
        CVector max = colModel.m_boundBox.m_vecMax;

        CVector workVec = min;
        CVector v1 = matrix * workVec;

        workVec.z = max.z;
        CVector v2 = matrix * workVec;

        workVec = min;
        workVec.x = max.x;
        CVector v3 = matrix * workVec;

        workVec = min;
        workVec.y = max.y;
        CVector v4 = matrix * workVec;

        workVec = min;
        workVec.y = max.y;
        workVec.z = max.z;
        CVector v5 = matrix * workVec;

        workVec = min;
        workVec.x = max.x;
        workVec.z = max.z;
        CVector v6 = matrix * workVec;

        workVec = min;
        workVec.x = max.x;
        workVec.y = max.y;
        CVector v7 = matrix * workVec;

        workVec = max;
        CVector v8 = matrix * workVec;

        RenderLineWithClipping(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v1.x, v1.y, v1.z, v3.x, v3.y, v3.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v1.x, v1.y, v1.z, v4.x, v4.y, v4.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v5.x, v5.y, v5.z, v2.x, v2.y, v2.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v5.x, v5.y, v5.z, v8.x, v8.y, v8.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v5.x, v5.y, v5.z, v4.x, v4.y, v4.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v6.x, v6.y, v6.z, v2.x, v2.y, v2.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v6.x, v6.y, v6.z, v8.x, v8.y, v8.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v6.x, v6.y, v6.z, v3.x, v3.y, v3.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v7.x, v7.y, v7.z, v8.x, v8.y, v8.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v7.x, v7.y, v7.z, v3.x, v3.y, v3.z, 0xFFFFFFFF, 0xFFFFFFFF);
        RenderLineWithClipping(v7.x, v7.y, v7.z, v4.x, v4.y, v4.z, 0xFFFFFFFF, 0xFFFFFFFF);
    }

    if (ObjManager::m_pSelected && m_bDrawAxisLines)
    {
        static float length = 300.0f;
        RwV3d pos = ObjManager::m_pSelected->GetPosition().ToRwV3d();

        RenderLineWithClipping(pos.x - length, pos.y, pos.z, pos.x + length, pos.y, pos.z, 0xFF0000FF, 0xFF0000FF);
        RenderLineWithClipping(pos.x, pos.y - length, pos.z, pos.x, pos.y + length, pos.z, 0x00FF00FF, 0x00FF00FF);
        RenderLineWithClipping(pos.x, pos.y, pos.z - length, pos.x, pos.y, pos.z + length, 0x0000FFFF, 0x0000FFFF);
    }

    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)false);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true);
}

void ObjManager::Init()
{
    // ---------------------------------------------------
    // optimizations
    m_vecModelNames.reserve(50);
    m_pVecEntities.reserve(100);

    // ---------------------------------------------------
    // highlight selected object
    CdeclEvent <AddressList<0x53E222, H_CALL>, PRIORITY_AFTER,  ArgPickNone, void()>renderEffects;
    renderEffects +=[]()
    {
        HighlightSelectedObj(m_pSelected);
    };

    // ---------------------------------------------------
    /*
    * Store model names in a vector
    * FIXME: add support for modloader
    */
    static std::string lastIplName;
    static std::vector<std::pair<int, std::string>> tempVec;
    static bool skip;

    CdeclEvent <AddressList<0x5B8428, H_CALL>, PRIORITY_BEFORE,  ArgPick2N<char*, 0, char*, 1>, void(char *a1, char* a2)>openCall;
    openCall += [](char *a1, char *a2)
    {
        lastIplName = std::filesystem::path(a1).stem().string();
        if (lastIplName == "DEFAULT" || lastIplName == "vehicles") // these two has vehicle entries
        {
            skip = true;
        }
        else
        {
            if (tempVec.size() > 0)
            {
                m_vecModelNames.push_back({std::move(lastIplName), std::move(tempVec)});
            }
            skip = false;
        }
    };
    CdeclEvent <AddressList<0x5B85DD, H_CALL>, PRIORITY_AFTER,  ArgPickN<char*, 0>, void(char *str)>loadIdeEvent;
    loadIdeEvent += [](char *str)
    {
        if (!skip)
        {
            std::stringstream ss(str);
            int model, unk2;
            char dffName[32], txdName[32];
            float unk;
            sscanf(str, "%d %s %s %f %d", &model, dffName, txdName, &unk, &unk2);
            tempVec.push_back({model, std::string(dffName)});
            totalIDELinesLoaded++;
        }
    };
    // ---------------------------------------------------
}

std::string ObjManager::FindNameFromModel(int model)
{
    for (auto &data : m_vecModelNames)
    {
        for (auto &iplData : data.second)
        {
            if (iplData.first == model)
            {
                return iplData.second;
            }
        }
    }
    return "dummy";
}

void ObjManager::HighlightSelectedObj(CObject *pObj)
{
    if (!pObj || !Editor::m_bShowEditor)
    {
        return;
    }

    CMatrix *matrix = pObj->GetMatrix();
    if (!matrix)
    {
        return;
    }

    unsigned short index = pObj->m_nModelIndex;
    if (!CModelInfo::ms_modelInfoPtrs[index])
    {
        return;
    }

    CColModel *pColModel = CModelInfo::ms_modelInfoPtrs[index]->m_pColModel;
    if (!pColModel)
    {
        return;
    }

    DrawColModel(*matrix, *pColModel);
}

void ObjManager::ExData::SetRotation(CVector rot, bool quat)
{
    m_vecRot = rot;
    Command<Commands::SET_OBJECT_ROTATION>(handle, rot.x, rot.y, rot.z);

    if (quat)
    {
        if (!pObj->m_pRwObject)
        {
            pObj->CreateRwObject();
        }
        Command<Commands::GET_OBJECT_QUATERNION>(handle, &m_fQuat.x, &m_fQuat.y, &m_fQuat.z, &m_fQuat.w);
    }
}

CVector ObjManager::ExData::GetRotation()
{
    return m_vecRot;
}

ObjManager::ExData::QUAT ObjManager::ExData::GetQuat()
{
    return m_fQuat;
}

void ObjManager::ExData::SetQuat(ObjManager::ExData::QUAT quat)
{
    m_fQuat = quat;
}