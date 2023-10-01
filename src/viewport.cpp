#include "pch.h"
#include "viewport.h"
#include "utils/utils.h"
#include "objectmgr.h"
#include "interface.h"
#include "utils/hotkeys.h"
#include <CHud.h>
#include <CModelInfo.h>
#include <CStreaming.h>
#include <CTxdStore.h>
#include <CScene.h>
#include <CSprite.h>
#include <D3dx9math.h>

#define PLAYER_Z_OFFSET 0.0f
#define MOUSE_FACTOR_X 13.0f
#define MOUSE_FACTOR_Y 6.0f

// Backups for restoring later on editor exit
static BYTE m_bHudState;
static BYTE m_bRadarState;
static bool m_bNeverWanted;

size_t BrowserMgr::GetSelected() {
    return m_nSelectedID;
}

void BrowserMgr::SetSelected(int modelId) {
    /*
        Check if the model is available first!
        If not set model to -1
    */
    if (modelId > 0 && Command<Commands::IS_MODEL_IN_CDIMAGE>(modelId)) {
        CStreaming::RequestModel(modelId, PRIORITY_REQUEST);
        CStreaming::LoadAllRequestedModels(true);
    }

    m_nSelectedID = Command<Commands::IS_MODEL_AVAILABLE>(modelId) ? modelId : NULL;
}

ImVec2 ViewportMgr::GetSize() {
    return m_fSize;
}

// Thanks junior & Júlio César
void BrowserMgr::LoadModel(size_t model, RpClump *&pClump, RpAtomic *&pAtomic, RwFrame *&pFrame) {
    pClump = nullptr;
    pAtomic = nullptr;
    pFrame = nullptr;

    if (!Command<Commands::IS_MODEL_AVAILABLE>(model)) {
        CStreaming::RequestModel(model, PRIORITY_REQUEST);
        CStreaming::LoadAllRequestedModels(true);
    }

    CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(model);
    int rwModelType = modelInfo->GetRwModelType();
    if (rwModelType == 1) {
        RpAtomic *atomic = (RpAtomic *)modelInfo->m_pRwObject;
        if (atomic) {
            modelInfo->AddRef();

            pFrame = RwFrameCreate();
            pAtomic = RpAtomicClone(atomic);
            RpAtomicSetFrame(pAtomic, pFrame);
        }
    } else {
        pClump = (RpClump*)modelInfo->m_pRwObject;
        if (pClump) {
            modelInfo->AddRef();
            pClump = (RpClump *)reinterpret_cast<CClumpModelInfo*>(modelInfo)->CreateInstance();
        }
    }
}

ViewportMgr Viewport;
ViewportMgr::ViewportMgr() {
    Events::initGameEvent += [this]() {
        Viewport.m_nMoveSpeed = gConfig.Get("editor.moveSpeed", 1.0f);
     };
}

void ViewportMgr::Init() {
    static CVector tMouse;

    CPlayerPed *pPlayer = FindPlayerPed();
    int hPlayer = CPools::GetPedRef(pPlayer);
    CVector pos = pPlayer->GetPosition();

    Command<Commands::SET_EVERYONE_IGNORE_PLAYER>(0, true);
    m_bHudState = patch::Get<BYTE>(0xBA6769); // hud
    m_bRadarState = patch::Get<BYTE>(0xBA676C); // radar
    patch::Set<BYTE>(0xBA6769, 0); // hud
    patch::Set<BYTE>(0xBA676C, 2); // radar

    CPad::GetPad(0)->DisablePlayerControls = true;
    pPlayer->m_bIsVisible = true;

    Command<Commands::FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION>(hPlayer, true);
    Command<Commands::SET_CHAR_COLLISION>(hPlayer, false);
    Command<Commands::SET_LOAD_COLLISION_FOR_CHAR_FLAG>(hPlayer, false);

    m_fMousePos.x = pPlayer->GetHeading();
    m_fMousePos.y = 0;
    pos.z -= PLAYER_Z_OFFSET;
    pPlayer->SetPosn(pos);
    CWorld::Remove(pPlayer);
    TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 1000, true);
    Command<Commands::CAMERA_PERSIST_FOV>(true);

    m_bNeverWanted = patch::Get<bool>(0x969171, false);
    if (!m_bNeverWanted) {
        Call<0x4396C0>();
    }

    m_bInitialized = true;
}

void BrowserMgr::Process() {
    static bool bEventsInjected;

    if (!bEventsInjected) {
        Events::drawingEvent += []() {
            if (Viewport.Browser.m_bShown) {
                // -------------------------------------------------
                /*
                * Calculations for pos & size of modelbg
                * Similar to to InfoMenu does it
                */
                float width = screen::GetScreenWidth();
                float height = screen::GetScreenHeight();
                float menuWidth = width/5.0f;
                float frameHeight = ImGui::GetFrameHeight();

                CRect pos(0.0f, 0.0f, width, height);
                CSprite2d::DrawRect(pos, CRGBA(0, 0, 0, 255));
                // -------------------------------------------------
                Viewport.Browser.RenderModel();
            }
        };
        bEventsInjected = true;
    }

    // Change object browser values if it's active
    if (Viewport.Browser.m_bShown && Viewport.m_eState == eViewportState::View) {
        CVector mouseDelta;
        Command<Commands::GET_PC_MOUSE_MOVEMENT>(&mouseDelta.x, &mouseDelta.y);
        m_fRot.y += mouseDelta.y / MOUSE_FACTOR_Y;

        if (!m_bAutoRot) {
            m_fRot.x += mouseDelta.x / MOUSE_FACTOR_X;
        }

        if (CPad::NewMouseControllerState.wheelUp) {
            m_fScale += 0.05f;
            m_fScale = m_fScale > 5.0f ? 5.0f : m_fScale;
        }

        if (CPad::NewMouseControllerState.wheelDown) {
            m_fScale -= 0.05f;
            m_fScale = m_fScale < 0.0f ? 0.0f : m_fScale;
        }
        return;
    }
}

void BrowserMgr::RenderModel() {
    static RpClump *pRpClump;
    static RwFrame *pRwFrame;
    static RpAtomic *pRpAtomic;
    static size_t loadedModelId = NULL;

    size_t modelId = GetSelected();
    if (!modelId) {
        return;
    }

    if (loadedModelId != modelId) {
        CBaseModelInfo *modelInfo = (CBaseModelInfo *)CModelInfo::GetModelInfo(loadedModelId);
        if (modelInfo) modelInfo->RemoveRef();
        if (pRpClump) RpClumpDestroy(pRpClump);
        if (pRpAtomic) RpAtomicDestroy(pRpAtomic);
        if (pRwFrame) RwFrameDestroy(pRwFrame);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(loadedModelId);
        LoadModel(modelId, pRpClump, pRpAtomic, pRwFrame);
        loadedModelId = modelId;
    }

    static float rotation = 0.0f;
    static RwRGBAReal AmbientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    static RwV3d pos = { 0.f, 0.05f, 2.0f };

    float f = m_fScale * 0.1f;
    RwV3d size = {f, f, f};
    static RwV3d axis1 = { 1.0f, 0.0f, 0.0f };
    static RwV3d axis2 = { 0.0f, 0.0f, 1.0f };
    static UINT32 timer = 0;

    if (pRpClump) {
        pRwFrame = (RwFrame*)pRpClump->object.parent;
    }

    if (m_bAutoRot && (CTimer::m_snTimeInMilliseconds - timer > 7)) {
        rotation += 1.0f;
        if (rotation > 360.0f)
            rotation -= 360.0f;
        timer = CTimer::m_snTimeInMilliseconds;
    }

    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)true);

    RwFrameTransform(pRwFrame, &GetObjectParent (&Scene.m_pRwCamera->object.object)->modelling, rwCOMBINEREPLACE);
    RwFrameTranslate(pRwFrame, &pos, rwCOMBINEPRECONCAT);
    RwFrameScale(pRwFrame, &size, rwCOMBINEPRECONCAT);
    RwFrameRotate(pRwFrame, &axis1, -90.0f + m_fRot.y, rwCOMBINEPRECONCAT);
    RwFrameRotate(pRwFrame, &axis2, rotation + + m_fRot.x, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(pRwFrame);
    SetAmbientColours(&AmbientColor);

    if (pRpClump) {
        RpClumpRender(pRpClump);
    } else {
        if (pRpAtomic) pRpAtomic->renderCallBack(pRpAtomic);
    }
}

void ViewportMgr::SetCameraPosn(const CVector &pos) {
    m_fCamPos = pos;

    /*
    * Since we're actually setting the player's position,
    * we need to add the offset to it
    */
    m_fCamPos.z -= PLAYER_Z_OFFSET;
    m_bCamUpdated = true;
}

void ViewportMgr::DrawHoverMenu() {
    if (!Interface.m_bShowHoverMenu || m_eState != eViewportState::Edit || !m_bHovered || Browser.m_bShown) {
        return;
    }

    CVector worldPos;
    if (Utils::TraceEntity(m_HoveredEntity, worldPos)) {
        ImGui::BeginTooltip();
        int model = m_HoveredEntity->m_nModelIndex;
        if (m_HoveredEntity->m_nType == ENTITY_TYPE_OBJECT
                || m_HoveredEntity->m_nType == ENTITY_TYPE_BUILDING) {
            static int bmodel = 0;
            static std::string name = "";

            // lets not go over 20000 models each frame
            if (bmodel != model) {
                name = ObjectMgr::FindNameFromModel(model);
                bmodel = model;
            }

            ImGui::Text("Name: %s", name.c_str());
        }
        std::string type = "";

        if (m_HoveredEntity->m_nType == ENTITY_TYPE_OBJECT) {
            type = "Dynamic";
        }
        if (m_HoveredEntity->m_nType == ENTITY_TYPE_BUILDING) {
            type = "Static";
        }

        ImGui::Text("Model: %d (%s)", model, type.c_str());
        ImGui::EndTooltip();
    }
}

void ViewportMgr::Cleanup() {
    CPlayerPed *pPlayer = FindPlayerPed(-1);
    CEntity* pPlayerEntity = FindPlayerEntity(-1);
    CVector pos = pPlayer->GetPosition();
    int hPlayer = CPools::GetPedRef(pPlayer);

    Command<Commands::SET_EVERYONE_IGNORE_PLAYER>(0, false);
    patch::Set<BYTE>(0xBA6769, m_bHudState); // hud
    patch::Set<BYTE>(0xBA676C, m_bRadarState); // radar
    CPad::GetPad(0)->DisablePlayerControls = false;

    pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 100.0f, nullptr, &pPlayerEntity) + 1.0f;
    pPlayer->SetPosn(pos);
    CWorld::Add(pPlayer);

    Command<Commands::FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION>(hPlayer, false);
    Command<Commands::SET_CHAR_COLLISION>(hPlayer, true);
    Command<Commands::SET_LOAD_COLLISION_FOR_CHAR_FLAG>(hPlayer, true);

    Command<Commands::CAMERA_PERSIST_FOV>(false);
    Command<Commands::RESTORE_CAMERA_JUMPCUT>();

    if (!m_bNeverWanted) {
        Call<0x4396C0>();
    }

    m_eState = eViewportState::Edit;
    Browser.m_bShown = false;
    Browser.m_bShowNextFrame = false;
    m_bInitialized = false;
}

static void my_PerspectiveFOV(float fov, float aspect, float _near, float _far, float* mret) {
    float D2R = 3.1416f / 180.0f;
    float yScale = 1.0 / tan(D2R * fov / 2);
    float xScale = yScale / aspect;
    float nearmfar = _near - _far;

    float m[] = {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, (_far + _near) / nearmfar, -1,
        0, 0, 2*_far*_near / nearmfar, 0
    };
    memcpy(mret, m, sizeof(float)*16);
}

void ViewportMgr::DrawOverlay() {
    // -------------------------------------------------
    /*
    * Calculations for pos & size of viewport
    * Similar to to InfoMenu does it
    */
    float width = screen::GetScreenWidth();
    float height = screen::GetScreenHeight();
    float menuWidth = width/5.0f;
    float frameHeight = ImGui::GetFrameHeight();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoMove
                             + ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowBgAlpha(0.0f); // and ofcource it needs to be transarent
    ImGui::SetNextWindowPos(ImVec2(0.0f, frameHeight));
    m_fSize = ImVec2(width-menuWidth, height - frameHeight);
    ImGui::SetNextWindowSize(m_fSize);

    if (ImGui::Begin("ViewPort", NULL, flags)) {
        m_bHovered = ImGui::IsWindowHovered();

        // if (ObjManager::m_pSelected)
        // {
        // 	ImGuizmo::BeginFrame();
        // 	ImGuizmo::SetRect(0, 0, screen::GetScreenWidth(), screen::GetScreenHeight());

        // 	CMatrix projectionMatrix;
        // 	my_PerspectiveFOV(TheCamera.FindCamFOV(), (float)screen::GetScreenWidth() / (float)screen::GetScreenHeight(), TheCamera.m_pRwCamera->nearPlane, TheCamera.m_pRwCamera->farPlane, (float*)&projectionMatrix);
        // 	CMatrix objMat = *ObjManager::m_pSelected->GetMatrix();
        // 	RwMatrix viewMat = TheCamera.m_pRwCamera->viewMatrix;


        // 	ImGuizmo::Manipulate((float*)&viewMat, (float*)&projectionMatrix, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, (float*)&objMat);
        // }

        // TODO: do popup menu checks here?
        Interface.DrawContextMenu();
        ImGui::End();
    }
}

static void QuickObjectCreatePopup() {
    static int modelId = 620;
    static std::string modelName = ObjectMgr::FindNameFromModel(modelId);

    ImGui::Text("Name: %s", modelName.c_str());
    if (ImGui::InputInt("Model", &modelId)) {
        modelName = ObjectMgr::FindNameFromModel(modelId);
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
        Command<Commands::CREATE_OBJECT>(modelId, Viewport.m_fWorldPos.x,
                                         Viewport.m_fWorldPos.y, Viewport.m_fWorldPos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(modelId);

        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = ObjectMgr::m_objData.Get(pEntity);
        data.m_modelName = modelName;

        ObjectMgr::m_pPlacedObjs.push_back(pEntity);
        ObjectMgr::m_pSelected = pEntity;

        Interface.m_PopupMenu.m_bShow = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Find models", Utils::GetSize(2))) {
        ShellExecute(NULL, "open", "https://dev.prineside.com/en/gtasa_samp_model_id/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    if (ImGui::Button("Open object browser", Utils::GetSize())) {
        Viewport.m_eState = eViewportState::Browser;
        Interface.m_PopupMenu.m_bShow = false;
        Viewport.Browser.m_bShowNextFrame = true;
    }
}

static void ContextMenu_NewObject() {
    CEntity *pEntity;

    if (Utils::TraceEntity(pEntity, Viewport.m_fWorldPos)) {
        Interface.m_PopupMenu.m_bShow = true;
        Interface.m_PopupMenu.m_pFunc = QuickObjectCreatePopup;
        Interface.m_PopupMenu.m_Title = "Quick object creator";

    }
}

static void ContextMenu_SnapToGround() {
    CVector objPos = ObjectMgr::m_pSelected->GetPosition();
    int hObj = CPools::GetObjectRef(ObjectMgr::m_pSelected);
    float offZ = objPos.z - ObjectMgr::GetBoundingBoxGroundZ(ObjectMgr::m_pSelected);
    objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
    Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos.x, objPos.y, objPos.z);
}

static void ContextMenu_Copy() {
    if (Viewport.m_HoveredEntity) {
        ObjectMgr::ClipBoard::m_nModel = Viewport.m_HoveredEntity->m_nModelIndex;

        CVector &rot = ObjectMgr::ClipBoard::m_vecRot;
        // Store rotation
        CallMethod<0x59A840, int>((int)Viewport.m_HoveredEntity->GetMatrix(),
                                  &rot.x, &rot.y, &rot.z, 0); //void __thiscall CMatrix::ConvertToEulerAngles(CMatrix *this, float *pX, float *pY, float *pZ, unsigned int flags)

        rot.x = RAD_TO_DEG(rot.x);
        rot.y = RAD_TO_DEG(rot.y);
        rot.z = RAD_TO_DEG(rot.z);

        // 0 -> 360
        Utils::GetDegreeInRange(&rot.x);
        Utils::GetDegreeInRange(&rot.y);
        Utils::GetDegreeInRange(&rot.z);
        CHud::SetHelpMessage("Object Copied", false, false, false);
    }
}

static void ContextMenu_Paste() {
    if (!ObjectMgr::ClipBoard::m_nModel) {
        return;
    }

    CEntity *pEntity;
    CVector pos;
    if (Command<Commands::IS_MODEL_AVAILABLE>(ObjectMgr::ClipBoard::m_nModel)
            && Utils::TraceEntity(pEntity, pos)) {
        int hObj;
        Command<Commands::REQUEST_MODEL>(ObjectMgr::ClipBoard::m_nModel);
        Command<Commands::LOAD_ALL_MODELS_NOW>();
        Command<Commands::CREATE_OBJECT>(ObjectMgr::ClipBoard::m_nModel, pos.x, pos.y, pos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(ObjectMgr::ClipBoard::m_nModel);

        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = ObjectMgr::m_objData.Get(pEntity);
        data.m_modelName = ObjectMgr::FindNameFromModel(ObjectMgr::ClipBoard::m_nModel);

        if (Interface.m_bRandomRot) {
            ObjectMgr::ClipBoard::m_vecRot.x = Random(Interface.m_RandomRotX[0], Interface.m_RandomRotX[1]);
            ObjectMgr::ClipBoard::m_vecRot.y = Random(Interface.m_RandomRotY[0], Interface.m_RandomRotY[1]);
            ObjectMgr::ClipBoard::m_vecRot.z = Random(Interface.m_RandomRotZ[0], Interface.m_RandomRotZ[1]);
        }

        data.SetRotation(ObjectMgr::ClipBoard::m_vecRot);

        ObjectMgr::m_pPlacedObjs.push_back(pEntity);
        ObjectMgr::m_pSelected = pEntity;
    }
}

static void ContextMenu_Delete() {
    if (ObjectMgr::m_pSelected) {
        ObjectMgr::m_pSelected->Remove();
        ObjectMgr::m_pPlacedObjs.erase(std::remove(ObjectMgr::m_pPlacedObjs.begin(),
                                        ObjectMgr::m_pPlacedObjs.end(), ObjectMgr::m_pSelected), ObjectMgr::m_pPlacedObjs.end());

        ObjectMgr::m_pSelected = nullptr;
    }
}

void ContextMenu_Viewport(std::string& root, std::string& key, std::string& value) {
    if (Viewport.Browser.m_bShown || Viewport.m_eState != eViewportState::Edit) {
        return;
    }

    if (ImGui::MenuItem("New object")) {
        ContextMenu_NewObject();
    }

    if (ImGui::MenuItem("Add to favourites")) {
        int model = Viewport.m_HoveredEntity->m_nModelIndex;
        std::string keyName = std::to_string(model) + " - " + ObjectMgr::FindNameFromModel(model);
        Interface.m_favData.m_pData->Set(std::format("All.{}", keyName).c_str(), model);
        Interface.m_favData.m_pData->Save();
        CHud::SetHelpMessage("Added to favourites", false, false, false);
        Interface.m_ContextMenu.m_bShow = false;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Snap to ground")) {
        ContextMenu_SnapToGround();
        Interface.m_ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Copy")) {
        ContextMenu_Copy();
        Interface.m_ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Paste")) {
        ContextMenu_Paste();
        Interface.m_ContextMenu.m_bShow = false;
    }

    if (ImGui::MenuItem("Delete")) {
        ContextMenu_Delete();
        Interface.m_ContextMenu.m_bShow = false;
    }
    ImGui::Separator();
}


void ViewportMgr::Process() {
    if (!m_bInitialized) {
        Init();
    }

    if (Interface.m_bShowGUI) {
        DrawOverlay();
        ProcessInputs();
        DrawHoverMenu();

        if (m_bHovered && ImGui::IsMouseClicked(1)) {
            Interface.m_ContextMenu.m_bShow = true;
            Interface.m_ContextMenu.m_pFunc = ContextMenu_Viewport;
            Interface.m_ContextMenu.m_Root = "";
            Interface.m_ContextMenu.m_Key = "";
            Interface.m_ContextMenu.m_Val = "";
        }
    }

    Browser.Process();

    // -------------------------------------------------
    // vars
    int delta = (CTimer::m_snTimeInMillisecondsNonClipped -
                 CTimer::m_snPreviousTimeInMillisecondsNonClipped);

    int ratio = 1 / (1 + (delta * m_nMoveSpeed));
    float speed = (m_nMoveSpeed + m_nMoveSpeed * ratio * delta);

    CPlayerPed* pPlayer = FindPlayerPed(-1);
    int hPlayer = CPools::GetPedRef(pPlayer);
    CVector pos = pPlayer->GetPosition();
    // -------------------------------------------------
    // Check if camera position was updated externally
    // If it was then change to new values
    if (m_bCamUpdated) {
        pos = m_fCamPos;
        m_bCamUpdated = false;
    }

    if (Interface.m_bInputLocked) {
        return;
    }

    if (viewportSwitchKey.Pressed()) {
        if(m_eState == eViewportState::Edit) {
            m_eState = eViewportState::View;
            D3dHook::SetMouseState(false);
        } else {
            D3dHook::SetMouseState(true);
            m_eState = eViewportState::Edit;
        }
    }

    if (KeyPressed(VK_LSHIFT)) {
        speed *= 2;
    }

    // Temp fix for a camera rotation bug switching from Edit -> View
    // Skipping 2 frames to fix this issue for now
    // TODO: FIX
    static size_t skippedFrames;
    if(m_eState == eViewportState::View) {
        if (skippedFrames > 1) {
            CVector mouseDelta;
            Command<Commands::GET_PC_MOUSE_MOVEMENT>(&mouseDelta.x, &mouseDelta.y);
            m_fMousePos.x -= mouseDelta.x / MOUSE_FACTOR_X;
            m_fMousePos.y += mouseDelta.y / MOUSE_FACTOR_Y;
        } else {
            ++skippedFrames;
        }
    } else {
        skippedFrames = 0;
    }

    m_fMousePos.x -= m_fMousePos.x > 360.0f ? 360.0f : 0.0f;
    m_fMousePos.x += m_fMousePos.x < 0.0f ? 360.0f : 0.0f;
    m_fMousePos.y += m_fMousePos.y > 150.0f ? 150.0f : 0.0f;
    m_fMousePos.y += m_fMousePos.y < -150.0f ? -150.0f : 0.0f;

    if (KeyPressed(VK_LCONTROL)) {
        speed /= 2;
    }

    if (KeyPressed(VK_LSHIFT)) {
        speed *= 2;
    }

    if (KeyPressed(VK_KEY_W) || KeyPressed(VK_KEY_S)) {
        if (KeyPressed(VK_KEY_S)) {
            speed *= -1;
        }

        float angle;
        Command<Commands::GET_CHAR_HEADING>(hPlayer, &angle);
        angle += 5.0f; // fix camera being slightly off
        pos.x += speed * cos(angle * 3.14159f / 180.0f);
        pos.y += speed * sin(angle * 3.14159f / 180.0f);
        pos.z += speed * 2 * sin(m_fMousePos.y / 3 * 3.14159f / 180.0f);
    }

    if (KeyPressed(VK_KEY_A) || KeyPressed(VK_KEY_D)) {
        if (KeyPressed(VK_KEY_A)) {
            speed *= -1;
        }

        float angle;
        Command<Commands::GET_CHAR_HEADING>(hPlayer, &angle);
        angle -= 90;

        pos.x += speed * cos(angle * 3.14159f / 180.0f);
        pos.y += speed * sin(angle * 3.14159f / 180.0f);
    }

    // -------------------------------------------------
    // Calcualte the zoom here
    if (m_eState == eViewportState::View) {
        if (CPad::NewMouseControllerState.wheelUp) {
            if (m_fFOV > 10.0f) {
                m_fFOV -= 2.0f;
            }

            TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 250, true);
            Command<Commands::CAMERA_PERSIST_FOV>(true);
        }

        if (CPad::NewMouseControllerState.wheelDown) {
            if (m_fFOV < 115.0f) {
                m_fFOV += 2.0f;
            }

            TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 250, true);
            Command<Commands::CAMERA_PERSIST_FOV>(true);
        }
    }
    // -------------------------------------------------

    Command<Commands::SET_CHAR_HEADING>(hPlayer, m_fMousePos.x);
    Command<Commands::ATTACH_CAMERA_TO_CHAR>(hPlayer, 0.0, 0.0, PLAYER_Z_OFFSET, 90.0, 0.0, m_fMousePos.y, 0.0, 2);
    pPlayer->SetPosn(pos);
}

void ViewportMgr::ProcessInputs() {
    if (!m_bHovered) {
        return;
    }

    // -------------------------------------------------
    // get object selection
    if (ImGui::IsMouseDoubleClicked(0)) {
        CEntity *pEntity;
        CVector pos;

        if (Utils::TraceEntity(pEntity, pos)) {
            ObjectMgr::m_pSelected = nullptr;
            for (auto &ent : ObjectMgr::m_pPlacedObjs) {
                if (ent == pEntity) {
                    ObjectMgr::m_pSelected = (CObject*)pEntity;
                    break;
                }
            }
        }
    }

    // -------------------------------------------------
    // Process object movement
    static int hObj = NULL;
    static CVector off; // offset between the mouse and the object pivot

    if (!(Interface.m_PopupMenu.m_bShow && Interface.m_ContextMenu.m_bShow)) {
        // -------------------------------------------------
        // X, Y, Z axis movement
        // TODO: Z axis is kinda buggy

        if (!Browser.m_bShown) {
            static bool bObjectBeingDragged;

            if (ImGui::IsMouseDown(0) && ObjectMgr::m_pSelected) {
                CEntity *pEntity;
                static CVector pos, off;
                bool bFound = Utils::TraceEntity(pEntity, pos);

                if (bFound) {
                    if (bObjectBeingDragged) {
                        auto &data = ObjectMgr::m_objData.Get(ObjectMgr::m_pSelected);
                        CVector objPos = CVector(pos.x - off.x, pos.y - off.y, pos.z - off.z);

                        if (Interface.m_bAutoSnapToGround) {
                            float offZ = objPos.z - ObjectMgr::GetBoundingBoxGroundZ(ObjectMgr::m_pSelected);
                            objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
                            off.z = pos.z - objPos.z;
                        }

                        Command<Commands::SET_OBJECT_COORDINATES>(data.handle, objPos.x, objPos.y, objPos.z);
                    } else {
                        if (pEntity == ObjectMgr::m_pSelected) {
                            off = pos - pEntity->GetPosition();
                            bObjectBeingDragged = true;
                        }
                    }
                }
            } else {
                if (bObjectBeingDragged
                        && Interface.m_bAutoSnapToGround && ObjectMgr::m_pSelected) {
                    auto &data = ObjectMgr::m_objData.Get(ObjectMgr::m_pSelected);
                    CVector pos = ObjectMgr::m_pSelected->GetPosition();
                    float offZ = pos.z - ObjectMgr::GetBoundingBoxGroundZ(ObjectMgr::m_pSelected);
                    pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 100.0f, nullptr, nullptr) + offZ;
                    Command<Commands::SET_OBJECT_COORDINATES>(data.handle, pos.x, pos.y, pos.z);
                }
                bObjectBeingDragged = false;
            }
        }

        // -------------------------------------------------
        // Z axis movement
        ImGuiIO &io = ImGui::GetIO();
        float wheel = io.MouseWheel;
        if (wheel && ObjectMgr::m_pSelected && m_eState == eViewportState::Edit) {
            if (KeyPressed(VK_LCONTROL)) {
                auto &data = ObjectMgr::m_objData.Get(ObjectMgr::m_pSelected);
                CVector rot = data.GetRotation();
                rot.z += 3*wheel;

                if (rot.z > 360.0f) {
                    rot.z -= 360.0f;
                }

                if (rot.z < 0.0f) {
                    rot.z += 360.0f;
                }

                data.SetRotation(rot);
            } else {
                CVector objPos = ObjectMgr::m_pSelected->GetPosition();
                Command<Commands::SET_OBJECT_COORDINATES>(CPools::GetObjectRef(ObjectMgr::m_pSelected),
                        objPos.x, objPos.y, objPos.z + wheel/3);
            }
        }


        if (!Interface.m_bInputLocked) {
            // -------------------------------------------------
            // Context menu shortcuts
            if (newObjKey.Pressed()) {
                ContextMenu_NewObject();
            }
            if (copyKey.Pressed()) {
                ContextMenu_Copy();
            }
            if(pasteKey.Pressed()) {
                ContextMenu_Paste();
            }
            if(deleteKey.Pressed()) {
                ContextMenu_Delete();
            }
            if(snapKey.Pressed()) {
                ContextMenu_SnapToGround();
            }
            // -------------------------------------------------
        }
    } else {
        hObj = NULL;
    }
    // -------------------------------------------------
}