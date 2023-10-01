#include "pch.h"
#include "filemgr.h"
#include "objmanager.h"
#include "utils/utils.h"
#include "defines.h"
#include <fstream>
#include <CHud.h>

void FileMgr::ImportIDE(std::string path, bool logImports) {
    if (std::filesystem::exists(path)) {
        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
            if (dirEntry.path().extension() == ".ide") {
                std::ifstream file(dirEntry.path());
                std::string line;
                std::vector<std::pair<int, std::string>> temp;

                while (std::getline(file, line)) {
                    if (line.starts_with("#")) {
                        continue;
                    }

                    if (logImports) {
                        Log::Print<eLogLevel::Debug>("Parsing line: %s\n", line);
                    }

                    int model, unk2;
                    char dffName[32], txdName[32];
                    float unk;
                    int rtn = sscanf(line.c_str(), "%d, %s, %s, %f, %d", &model, dffName, txdName, &unk, &unk2);

                    if (rtn == 2) {
                        char *c = strchr(dffName, ',');
                        if (c) {
                            *c = '\0';
                        }
                        temp.push_back({model, std::string(dffName)});
                    }
                }

                ObjManager::m_vecModelNames.push_back({dirEntry.path().stem().string(), std::move(temp)});
                ObjManager::m_nTotalIDELine++;
            }
        }
    }
}

void FileMgr::ImportIPL(std::string fileName, bool logImports) {
    static int counter = 0;
    std::fstream file;
    std::string fullPath = PLUGIN_PATH((char*)FILE_NAME"/") + fileName;
    file.open(fullPath.c_str(), std::ios::in);
    std::string line;

    int model, interior, unk;
    char modelName[32];
    CVector pos;
    float rx, ry, rz, rw;

    while (getline(file, line)) {
        for(char& c : line ) {
            if( c == ',' ) c = ' ' ;
        }

        if (sscanf(line.c_str(), "%d %s %d %f %f %f %f %f %f %f %d", &model, modelName, &interior,
                   &pos.x, &pos.y, &pos.z, &rx, &ry, &rz, &rw, &unk) == 11) {
            if (logImports) {
                Log::Print<eLogLevel::Debug>("Parsing line: %s\n", line);
            }

            int hObj;
            Command<Commands::REQUEST_MODEL>(model);
            Command<Commands::LOAD_ALL_MODELS_NOW>();
            Command<Commands::CREATE_OBJECT>(model, pos.x, pos.y, pos.z, &hObj);

            CObject *pObj = CPools::GetObject(hObj);
            auto &data = ObjManager::m_objData.Get(pObj);
            Command<Commands::SET_OBJECT_QUATERNION>(hObj, rx, ry, rz, rw);

            // Store rotation
            CVector rot;
            CallMethod<0x59A840, int>((int)pObj->GetMatrix(), &rot.x, &rot.y, &rot.z, 0); //void __thiscall CMatrix::ConvertToEulerAngles(CMatrix *this, float *pX, float *pY, float *pZ, unsigned int flags)

            rot.x = RAD_TO_DEG(rot.x);
            rot.y = RAD_TO_DEG(rot.y);
            rot.z = RAD_TO_DEG(rot.z);

            // 0 -> 360
            Utils::GetDegreeInRange(&rot.x);
            Utils::GetDegreeInRange(&rot.y);
            Utils::GetDegreeInRange(&rot.z);
            data.SetRotation(rot, false);
            data.SetQuat({rx, ry, rz, rw});
            data.m_modelName = ObjManager::FindNameFromModel(pObj->m_nModelIndex);

            // Setting quat messes with z coord?
            Command<Commands::SET_OBJECT_COORDINATES>(hObj, pos.x, pos.y, pos.z);
            Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(model);
            ObjManager::m_pPlacedObjs.push_back(CPools::GetObject(hObj));
        }
    }
    CHud::SetHelpMessage("IPL imported", false, false, false);
}

void FileMgr::ExportIPL(const char* fileName) {
    std::fstream file;
    file.open(std::string(PLUGIN_PATH((char*)FILE_NAME"/")) + fileName, std::ios::out);
    file << "# Generated using Map Editor by Grinch_\ninst" << std::endl;
    for (CObject *pObj : ObjManager::m_pPlacedObjs) {
        int model = pObj->m_nModelIndex;
        auto &data = ObjManager::m_objData.Get(pObj);
        CVector pos;
        Command<Commands::GET_OBJECT_COORDINATES>(data.handle, &pos.x, &pos.y, &pos.z);
        auto quat = data.GetQuat();

        // These don't work
        // Command<Commands::GET_OBJECT_QUATERNION>(...)

        // if (pData->m_pRwObject)
        // {
        //     RwMatrix modelingMatrix = reinterpret_cast<RwFrame*>(pData->m_pRwObject->parent)->modelling;
        //     quat.Set(modelingMatrix);
        // }

        file << std::format("{}, {}, 0, {}, {}, {},  {}, {}, {}, {}, -1",
                            model, ObjManager::FindNameFromModel(model), pos.x, pos.y, pos.z,
                            quat.x,  quat.y,  quat.z, quat.w)
             << std::endl;
    }
    file << "end" << std::endl;
    file.close();
    CHud::SetHelpMessage("IPL exported", false, false, false);
}