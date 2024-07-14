#include "pch.h"
#include "utils/utils.h"
#include "viewport.h"
#include "entitymgr.h"
#include "interface.h"
#include "contextmenus.h"
#include "popups.h"

ContextMenus ContextMenu;

static void QuickObjectCreatePopup() {
    static int modelId = 620;
    static std::string modelName = EntMgr.FindNameFromModel(modelId);

    ImGui::Text("Name: %s", modelName.c_str());
    if (ImGui::InputInt("Model", &modelId)) {
        modelName = EntMgr.FindNameFromModel(modelId);
    }
    if (KeyPressed(VK_RETURN)) {
        goto create_object;
    }

    ImGui::Spacing();
    if (ImGui::Button("Create", Utils::GetSize(2))) {
create_object:
        int hObj;
        Command<Commands::REQUEST_MODEL>(modelId);
        Command<Commands::LOAD_ALL_MODELS_NOW>();
        Command<Commands::CREATE_OBJECT>(modelId, Viewport.m_fCursorWorldPos.x,
                                         Viewport.m_fCursorWorldPos.y, Viewport.m_fCursorWorldPos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(modelId);

        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = EntMgr.m_Info.Get(pEntity);
        data.m_sModelName = modelName;

        EntMgr.m_pPlaced.push_back(pEntity);
        EntMgr.m_pSelected = pEntity;

        Popup.m_bShow = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Find models", Utils::GetSize(2))) {
        ShellExecute(NULL, "open", "https://dev.prineside.com/en/gtasa_samp_model_id/", NULL, NULL, SW_SHOWNORMAL);
    } 
    ImGui::Spacing();
    if (ImGui::Button("Open object browser", Utils::GetSize())) {
        Popup.m_bShow = false;
        Viewport.m_Renderer.m_bShowNextFrame = true;
    }
}

void ContextMenu_RegularNoRemoveMenu(std::string& root, std::string& key, std::string& value) {
    if (ImGui::MenuItem("Add to favourites")) {
        Action_AddToFavourites(std::stoi(value));
    };

    if (ImGui::MenuItem("Copy")) {
        EntMgr.ClipBoard.m_nModel = std::stoi(value);
        CHud::SetHelpMessage("Object Copied", false, false, false);
    };
}

void ContextMenu_NewObject() {
    CEntity *pEntity;

    if (Utils::TraceEntity(pEntity, Viewport.m_fCursorWorldPos)) {
        Popup.m_bShow = true;
        Popup.m_pFunc = QuickObjectCreatePopup;
        Popup.m_Title = "Quick object creator";

    }
}

void ContextMenu_SnapToGround() {
    if (EntMgr.m_pSelected) {
        CVector objPos = EntMgr.m_pSelected->GetPosition();
        int hObj = CPools::GetObjectRef(EntMgr.m_pSelected);
        float offZ = objPos.z - EntMgr.GetBoundingBoxGroundZ(EntMgr.m_pSelected);
        objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
        Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos.x, objPos.y, objPos.z);
    }
}

void ContextMenu_Copy() {
    if (Viewport.m_HoveredEntity) {
        EntMgr.ClipBoard.m_nModel = Viewport.m_HoveredEntity->m_nModelIndex;

        Viewport.m_HoveredEntity->GetOrientation(EntMgr.ClipBoard.m_Rot.x, EntMgr.ClipBoard.m_Rot.y, EntMgr.ClipBoard.m_Rot.z);
        EntMgr.ClipBoard.m_Rot.x = RAD_TO_DEG(EntMgr.ClipBoard.m_Rot.x);
        EntMgr.ClipBoard.m_Rot.y = RAD_TO_DEG(EntMgr.ClipBoard.m_Rot.y);
        EntMgr.ClipBoard.m_Rot.z = 360.0f - RAD_TO_DEG(EntMgr.ClipBoard.m_Rot.z);

        CHud::SetHelpMessage("Object Copied", false, false, false);
    }
}

void ContextMenu_Paste() {
    if (!EntMgr.ClipBoard.m_nModel) {
        return;
    }

    CEntity *pEntity;
    CVector pos;
    if (Command<Commands::IS_MODEL_AVAILABLE>(EntMgr.ClipBoard.m_nModel)
            && Utils::TraceEntity(pEntity, pos)) {
        int hObj;
        Command<Commands::REQUEST_MODEL>(EntMgr.ClipBoard.m_nModel);
        Command<Commands::LOAD_ALL_MODELS_NOW>();
        Command<Commands::CREATE_OBJECT>(EntMgr.ClipBoard.m_nModel, pos.x, pos.y, pos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(EntMgr.ClipBoard.m_nModel);

        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = EntMgr.m_Info.Get(pEntity);
        data.m_sModelName = EntMgr.FindNameFromModel(EntMgr.ClipBoard.m_nModel);

        if (Interface.m_bRandomRot) {
            EntMgr.ClipBoard.m_Rot.x = RandomNumberInRange(Interface.m_RandomRotX[0], Interface.m_RandomRotX[1]);
            EntMgr.ClipBoard.m_Rot.y = RandomNumberInRange(Interface.m_RandomRotY[0], Interface.m_RandomRotY[1]);
            EntMgr.ClipBoard.m_Rot.z = RandomNumberInRange(Interface.m_RandomRotZ[0], Interface.m_RandomRotZ[1]);
        }

        data.SetEuler(EntMgr.ClipBoard.m_Rot);

        EntMgr.m_pPlaced.push_back(pEntity);
        EntMgr.m_pSelected = pEntity;
    }
}

void Action_RemoveSelectedObject() {
    if (EntMgr.m_pSelected) {
        EntMgr.m_pSelected->Remove();
        EntMgr.m_pPlaced.erase(std::remove(EntMgr.m_pPlaced.begin(),
                                        EntMgr.m_pPlaced.end(), EntMgr.m_pSelected), EntMgr.m_pPlaced.end());

        EntMgr.m_pSelected = nullptr;
    }
}

void Action_MoveCamToObject(CObject *pObj) {
    CMatrix *matrix = pObj->GetMatrix();
    CColModel *pColModel = pObj->GetColModel();
    CVector min = pColModel->m_boundBox.m_vecMin;
    CVector max = pColModel->m_boundBox.m_vecMax;

    CVector vec = {(min.x + max.x)/2, (min.y + max.y)/2, max.z};
    vec = *matrix * vec;

    Viewport.SetCameraPosn(vec);
    EntMgr.m_pSelected = pObj;
}

void Action_AddToFavourites(int model) {
    std::string keyName = std::to_string(model) + " - " + EntMgr.FindNameFromModel(model);
    Interface.m_favData.m_pData->Set(std::format("Favourites.{}", keyName).c_str(), std::to_string(model));
    Interface.m_favData.m_pData->Save();
    Interface.m_favData.UpdateSearchList(true);
    CHud::SetHelpMessage("Added to favourites", false, false, false);
}

void ContextMenus::Draw() {
    if (m_bShow) {
        if (ImGui::BeginPopupContextWindow("ContextMenu")) {
            if (m_Key != "") {
                ImGui::Text(m_Key.c_str());
                ImGui::Separator();
            }

            m_pFunc(m_Root, m_Key, m_Val);

            if (ImGui::MenuItem("Close")) {
                m_bShow = false;
            }

            ImGui::EndPopup();
        }
    }
}

void ContextMenu_RegularMenu(std::string& root, std::string& key, std::string& value) {
    if (ImGui::MenuItem("Add to favourites")) {
        int model = ((CObject*)std::stoi(value))->m_nModelIndex;
        Action_AddToFavourites(model);
        ContextMenu.m_bShow = false;
    };

    if (ImGui::MenuItem("Copy")) {
        EntMgr.ClipBoard.m_nModel = std::stoi(value);
        CHud::SetHelpMessage("Object Copied", false, false, false);
        ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Remove")) {
        CObject *pObj = (CObject*)std::stoi(value);
        if (pObj) {
            pObj->Remove();
            EntMgr.m_pPlaced.erase(std::remove(EntMgr.m_pPlaced.begin(),
                                            EntMgr.m_pPlaced.end(), pObj), EntMgr.m_pPlaced.end());
        }
        ContextMenu.m_bShow = false;
    }
}

void ContextMenu_Viewport(std::string& root, std::string& key, std::string& value) {
    if (Viewport.m_Renderer.m_bShown || Viewport.m_eState != eViewportState::Edit) {
        return;
    }

    if (ImGui::MenuItem("New object")) {
        ContextMenu_NewObject();
        ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Add to favourites")) {
        int model = Viewport.m_HoveredEntity->m_nModelIndex;
        Action_AddToFavourites(model);
        ContextMenu.m_bShow = false;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Snap to ground", NULL, false, EntMgr.m_pSelected)) {
        ContextMenu_SnapToGround();
        ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Copy")) {
        ContextMenu_Copy();
        ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Paste", NULL, false, EntMgr.ClipBoard.m_nModel != -1)) {
        ContextMenu_Paste();
        ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Delete", NULL, false, EntMgr.m_pSelected)) {
        Action_RemoveSelectedObject();
        ContextMenu.m_bShow = false;
    }
    ImGui::Separator();
}