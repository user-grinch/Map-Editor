#include "pch.h"
#include "filemgr.h"
#include "entitymgr.h"
#include "utils/utils.h"
#include "defines.h"
#include <fstream>
#include <CHud.h>

void FileMgr::ImportIPL(std::string fileName, bool logImports) {
    std::fstream file;
    file.open(PLUGIN_PATH((char*)FILE_NAME"/") + fileName, std::ios::in);
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
            auto &data = EntMgr.m_Info.Get(pObj);
            data.SetQuat({{rx, ry, rz}, rw});
            data.m_sModelName = EntMgr.FindNameFromModel(pObj->m_nModelIndex);

            Command<Commands::SET_OBJECT_COORDINATES>(hObj, pos.x, pos.y, pos.z);
            Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(model);
            EntMgr.m_pPlaced.push_back(CPools::GetObject(hObj));
        }
    }
    CHud::SetHelpMessage("IPL imported", false, false, false);
}

void FileMgr::ExportIPL(const char* fileName) {
    std::fstream file;
    file.open(std::string(PLUGIN_PATH((char*)FILE_NAME"/")) + fileName, std::ios::out);
    file << "# Generated using Map Editor by Grinch_\ninst" << std::endl;
    for (CObject *pObj : EntMgr.m_pPlaced) {
        int model = pObj->m_nModelIndex;
        auto &data = EntMgr.m_Info.Get(pObj);
        CVector pos;
        Command<Commands::GET_OBJECT_COORDINATES>(data.m_nHandle, &pos.x, &pos.y, &pos.z);
        auto quat = data.GetQuat();
        file << std::format("{}, {}, 0, {}, {}, {},  {}, {}, {}, {}, -1", model, EntMgr.FindNameFromModel(model), pos.x, pos.y, pos.z,
            quat.imag.x,  quat.imag.y,  quat.imag.z, quat.real) << std::endl;
    }
    file << "end" << std::endl;
    file.close();
    CHud::SetHelpMessage("IPL exported", false, false, false);
}