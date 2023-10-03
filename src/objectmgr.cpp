#include "pch.h"
#include "editor.h"
#include "objectmgr.h"
#include "filemgr.h"
#include "interface.h"
#include "defines.h"
#include <CRenderer.h>
#include <CModelInfo.h>
#include <filesystem>

float ObjectMgr::GetBoundingBoxGroundZ(CObject *pObj) {
    if (!pObj) {
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

ObjectMgr ObjMgr;
ObjectMgr::ObjectMgr() {
    // ---------------------------------------------------
    // optimizations
    m_vecModelNames.reserve(50);
    m_pPlacedObjs.reserve(100);
    /*
    * Store model names in a vector
    * FIXME: add support for modloader
    */
    static std::string lastIdeName;
    static std::vector<std::pair<int, std::string>> tempVec;
    static bool skip;

    CdeclEvent <AddressList<0x5B8428, H_CALL>, PRIORITY_BEFORE,  ArgPick2N<char*, 0, char*, 1>, void(char *a1, char* a2)>openCall;
    openCall += [this](char *a1, char *a2) {
        lastIdeName = std::filesystem::path(a1).stem().string();
        if (lastIdeName == "DEFAULT" || lastIdeName == "vehicles" || lastIdeName == "peds") { // these two has vehicle entries
            skip = true;
        } else {
            if (tempVec.size() > 0) {
                m_vecModelNames.push_back({std::move(lastIdeName), std::move(tempVec)});
            }
            skip = false;
        }
    };

    CdeclEvent <AddressList<0x5B85DD, H_CALL>, PRIORITY_AFTER,  ArgPickN<char*, 0>, void(char *str)>loadIdeEvent;
    loadIdeEvent += [this](char *str) {
        if (!skip) {
            int model, unk2;
            char dffName[32], txdName[32];
            float unk;
            sscanf(str, "%d %s %s %f %d", &model, dffName, txdName, &unk, &unk2);
            tempVec.push_back({model, std::string(dffName)});
            m_nTotalIDELine++;
        }
    };

    // Modloader
    // We're only loading from ModLoader/MapEditor directory.
    FileMgr::ImportIDE(PLUGIN_PATH((char*)"\\modloader\\"FILE_NAME"\\"));
    // ---------------------------------------------------
}

std::string ObjectMgr::FindNameFromModel(int model) {
    for (auto &data : m_vecModelNames) {
        for (auto &iplData : data.second) {
            if (iplData.first == model) {
                return iplData.second;
            }
        }
    }
    return "dummy";
}

void ExData::SetRotation(CVector rot, bool quat) {
    m_vecRot = rot;
    Command<Commands::SET_OBJECT_ROTATION>(handle, rot.x, rot.y, rot.z);

    if (quat) {
        if (!pObj->m_pRwObject) {
            pObj->CreateRwObject();
        }
        Command<Commands::GET_OBJECT_QUATERNION>(handle, &m_fQuat.x, &m_fQuat.y, &m_fQuat.z, &m_fQuat.w);
    }
}

CVector ExData::GetRotation() {
    return m_vecRot;
}

ExData::QUAT ExData::GetQuat() {
    return m_fQuat;
}

void ExData::SetQuat(ExData::QUAT quat) {
    m_fQuat = quat;
}