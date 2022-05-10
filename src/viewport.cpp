#include "pch.h"
#include "viewport.h"
#include "utils.h"
#include "objmanager.h"
#include "interface.h"
#include "hotkeys.h"
#include <CHud.h>
#include <CModelInfo.h>
#include <CStreaming.h>
#include <CTxdStore.h>
#include <CScene.h>
#include <CSprite.h>
#include <D3dx9math.h>

void Viewport::Init()
{
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

    m_fTotalMouse.x = pPlayer->GetHeading();
    m_fTotalMouse.y = 0;
    pos.z -= PLAYER_Z_OFFSET;
    pPlayer->SetPosn(pos);
    CWorld::Remove(pPlayer);
    TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 1000, true);
    Command<Commands::CAMERA_PERSIST_FOV>(true);

    m_bNeverWanted = patch::Get<bool>(0x969171, false);
    if (!m_bNeverWanted)
    {
        Call<0x4396C0>();
    }

    static bool bEventsInjected;

    if (!bEventsInjected)
    {
        Events::drawingEvent += []()
        {
            if (Interface::Browser::m_bShown)
            {
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
                RenderObjectBrowserModel();
            }
        };
        bEventsInjected = true;
    }

    m_bInitialized = true;
}

// Thanks junior & Júlio César
void Viewport::LoadNewClump(int model, RpClump *&pClump, RpAtomic *&pAtomic, RwFrame *&pFrame)
{
    pClump = nullptr;
    pAtomic = nullptr;
    pFrame = nullptr;
    
    if (!Command<Commands::IS_MODEL_AVAILABLE>(model))
    {
        CStreaming::RequestModel(model, PRIORITY_REQUEST);
        CStreaming::LoadAllRequestedModels(true);
    }

    CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(model);
    int rwModelType = modelInfo->GetRwModelType();
    if (rwModelType == 1)
    {
        RpAtomic *atomic = (RpAtomic *)modelInfo->m_pRwObject;
        if (atomic)
        {
            modelInfo->AddRef();

            pFrame = RwFrameCreate();
            pAtomic = RpAtomicClone(atomic);
            RpAtomicSetFrame(pAtomic, pFrame);
        }
    }
    else
    {
        pClump = (RpClump*)modelInfo->m_pRwObject;
        if (pClump)
        {
            modelInfo->AddRef();
            pClump = (RpClump *)reinterpret_cast<CClumpModelInfo*>(modelInfo)->CreateInstance();
        }
    }
}

void Viewport::RenderObjectBrowserModel()
{
    static RpClump *pRpClump;
    static RwFrame *pRwFrame;
    static RpAtomic *pRpAtomic;
    static size_t loadedModelId = NULL;

    size_t modelId = Interface::Browser::GetSelected();
    if (!modelId)
    {
        return;
    }
    
    if (loadedModelId != modelId)
    {
        CBaseModelInfo *modelInfo = (CBaseModelInfo *)CModelInfo::GetModelInfo(loadedModelId);
        if (modelInfo) modelInfo->RemoveRef();
        if (pRpClump) RpClumpDestroy(pRpClump);
        if (pRpAtomic) RpAtomicDestroy(pRpAtomic);
        if (pRwFrame) RwFrameDestroy(pRwFrame);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(loadedModelId);
        LoadNewClump(modelId, pRpClump, pRpAtomic, pRwFrame);
        loadedModelId = modelId;
    }

    static float rotation = 0.0f;
    static RwRGBAReal AmbientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    static RwV3d pos = { 0.f, 0.05f, 2.0f };
    RwV3d size = { 0.1f*m_nRenderScale, 0.1f*m_nRenderScale,0.1f*m_nRenderScale};
    static RwV3d axis1 = { 1.0f, 0.0f, 0.0f };
    static RwV3d axis2 = { 0.0f, 0.0f, 1.0f };
    static UINT32 timer = 0;

    if (pRpClump)
    {
        pRwFrame = (RwFrame*)pRpClump->object.parent;
    }

    if (m_bObjBrowserAutoRot && (CTimer::m_snTimeInMilliseconds - timer > 7))
    {
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
    RwFrameRotate(pRwFrame, &axis1, -90.0f + m_vecRenderRot.y, rwCOMBINEPRECONCAT);
    RwFrameRotate(pRwFrame, &axis2, rotation + + m_vecRenderRot.x, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(pRwFrame);
    SetAmbientColours(&AmbientColor);

    if (pRpClump)
    {
        RpClumpRender(pRpClump);
    }
    else
    {
        if (pRpAtomic) pRpAtomic->renderCallBack(pRpAtomic);
    }
}

void Viewport::SetCameraPosn(const CVector &pos)
{
    m_fCameraPos = pos;

    /*
    * Since we're actually setting the player's position,
    * we need to add the offset to it
    */
    m_fCameraPos.z -= PLAYER_Z_OFFSET;
    m_bCameraPosWasUpdated = true;
}

void Viewport::DrawHoverMenu()
{
    if (!m_bShowHoverMenu || m_eViewportMode != EDIT_MODE
            || !m_bBeingHovered || Interface::Browser::m_bShown)
    {
        return;
    }

    CVector worldPos;
    if (Utils::TraceEntity(m_HoveredEntity, worldPos))
    {
        ImGui::BeginTooltip();
        int model = m_HoveredEntity->m_nModelIndex;
        if (m_HoveredEntity->m_nType == ENTITY_TYPE_OBJECT
                || m_HoveredEntity->m_nType == ENTITY_TYPE_BUILDING)
        {
            static int bmodel = 0;
            static std::string name = "";

            // lets not go over 20000 models each frame
            if (bmodel != model)
            {
                name = ObjManager::FindNameFromModel(model);
                bmodel = model;
            }

            ImGui::Text("Name: %s", name.c_str());
        }
        std::string type = "";

        if (m_HoveredEntity->m_nType == ENTITY_TYPE_OBJECT)
        {
            type = "Dynamic";
        }
        if (m_HoveredEntity->m_nType == ENTITY_TYPE_BUILDING)
        {
            type = "Static";
        }

        ImGui::Text("Model: %d (%s)", model, type.c_str());
        ImGui::EndTooltip();
    }
}

void Viewport::Process()
{
    if (!m_bInitialized)
    {
        Init();
    }

    // -------------------------------------------------
    // vars
    int delta = (CTimer::m_snTimeInMillisecondsNonClipped -
                 CTimer::m_snPreviousTimeInMillisecondsNonClipped);

    int ratio = 1 / (1 + (delta * m_nMul));
    int speed = m_nMul + m_nMul * ratio * delta;

    CPlayerPed* pPlayer = FindPlayerPed(-1);
    int hPlayer = CPools::GetPedRef(pPlayer);
    CVector pos = pPlayer->GetPosition();
    // -------------------------------------------------
    // Check if camera position was updated externally
    // If it was then change to new values
    if (m_bCameraPosWasUpdated)
    {
        pos = m_fCameraPos;
        m_bCameraPosWasUpdated = false;
    }

    if (Interface::m_bIsInputLocked)
    {
        return;
    }

    if (viewportSwitchKey.Pressed())
    {
        if(m_eViewportMode == EDIT_MODE)
        {
            m_eViewportMode = VIEW_MODE;
            D3dHook::SetMouseState(false);
        }
        else
        {
            D3dHook::SetMouseState(true);
            m_eViewportMode = EDIT_MODE;
        }
        m_bShowContextMenu = false;
    }

    // Change object browser values if it's active
    if (Interface::Browser::m_bShown
            && Viewport::m_eViewportMode == VIEW_MODE)
    {
        CVector mouseDelta;
        Command<Commands::GET_PC_MOUSE_MOVEMENT>(&mouseDelta.x, &mouseDelta.y);
        m_vecRenderRot.y += mouseDelta.y / MOUSE_FACTOR_Y;

        if (!m_bObjBrowserAutoRot)
        {
            m_vecRenderRot.x += mouseDelta.x / MOUSE_FACTOR_X;
        }

        if (CPad::NewMouseControllerState.wheelUp)
        {
            m_nRenderScale += 0.05f;
            if (m_nRenderScale > 5.0f)
            {
                m_nRenderScale = 5.0f;
            }
        }

        if (CPad::NewMouseControllerState.wheelDown)
        {
            m_nRenderScale -= 0.05f;

            if (m_nRenderScale < 0.0f)
            {
                m_nRenderScale = 0.0f;
            }
        }
        return;
    }

    if (KeyPressed(VK_LSHIFT))
    {
        speed *= 2;
    }

    if(m_eViewportMode == VIEW_MODE)
    {
        CVector mouseDelta;
        Command<Commands::GET_PC_MOUSE_MOVEMENT>(&mouseDelta.x, &mouseDelta.y);
        m_fTotalMouse.x -= mouseDelta.x / MOUSE_FACTOR_X;
        m_fTotalMouse.y += mouseDelta.y / MOUSE_FACTOR_Y;
    }

    if (m_fTotalMouse.x > 360.0f)
    {
        m_fTotalMouse.x -= 360.0f;
    }

    if (m_fTotalMouse.x < 0.0f)
    {
        m_fTotalMouse.x += 360.0f;
    }

    if (m_fTotalMouse.y > 150)
    {
        m_fTotalMouse.y = 150;
    }

    if (m_fTotalMouse.y < -150)
    {
        m_fTotalMouse.y = -150;
    }

    if (KeyPressed(VK_RCONTROL))
    {
        speed /= 2;
    }

    if (KeyPressed(VK_RSHIFT))
    {
        speed *= 2;
    }

    if (KeyPressed(VK_KEY_W) || KeyPressed(VK_KEY_S))
    {
        if (KeyPressed(VK_KEY_S))
        {
            speed *= -1;
        }

        float angle;
        Command<Commands::GET_CHAR_HEADING>(hPlayer, &angle);
        angle += 5.0f; // fix camera being slightly off
        pos.x += speed * cos(angle * 3.14159f / 180.0f);
        pos.y += speed * sin(angle * 3.14159f / 180.0f);
        pos.z += speed * 2 * sin(m_fTotalMouse.y / 3 * 3.14159f / 180.0f);
    }

    if (KeyPressed(VK_KEY_A) || KeyPressed(VK_KEY_D))
    {
        if (KeyPressed(VK_KEY_A))
        {
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
    if (Viewport::m_eViewportMode == VIEW_MODE)
    {
        if (CPad::NewMouseControllerState.wheelUp)
        {
            if (m_fFOV > 10.0f)
            {
                m_fFOV -= 2.0f;
            }

            TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 250, true);
            Command<Commands::CAMERA_PERSIST_FOV>(true);
        }

        if (CPad::NewMouseControllerState.wheelDown)
        {
            if (m_fFOV < 115.0f)
            {
                m_fFOV += 2.0f;
            }

            TheCamera.LerpFOV(TheCamera.FindCamFOV(), m_fFOV, 250, true);
            Command<Commands::CAMERA_PERSIST_FOV>(true);
        }
    }
    // -------------------------------------------------

    Command<Commands::SET_CHAR_HEADING>(hPlayer, m_fTotalMouse.x);
    Command<Commands::ATTACH_CAMERA_TO_CHAR>(hPlayer, 0.0, 0.0, PLAYER_Z_OFFSET, 90.0, 0.0, m_fTotalMouse.y, 0.0, 2);
    pPlayer->SetPosn(pos);
}

void Viewport::Shutdown()
{
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

    if (!m_bNeverWanted)
    {
        Call<0x4396C0>();
    }

    m_bInitialized = false;
}

static void my_PerspectiveFOV(float fov, float aspect, float _near, float _far, float* mret)
{
    float D2R = 3.1416f / 180.0f;
    float yScale = 1.0 / tan(D2R * fov / 2);
    float xScale = yScale / aspect;
    float nearmfar = _near - _far;

    float m[] =
    {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, (_far + _near) / nearmfar, -1,
        0, 0, 2*_far*_near / nearmfar, 0
    };
    memcpy(mret, m, sizeof(float)*16);
}

void Viewport::DrawOverlay()
{
    if (ImGui::IsMouseClicked(1))
    {
        m_bShowContextMenu = true;
    }

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
    m_fViewportSize = ImVec2(width-menuWidth, height - frameHeight);
    ImGui::SetNextWindowSize(m_fViewportSize);

    if (ImGui::Begin("ViewPort", NULL, flags))
    {
        m_bBeingHovered = ImGui::IsWindowHovered();

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
        if (m_bShowContextMenu)
        {
            ProcessContextMenu();
        }
        ImGui::End();
    }
}

static void ContextMenu_NewObject()
{
    CEntity *pEntity;

    if (Utils::TraceEntity(pEntity, Viewport::m_vecWorldPos))
    {
        Interface::m_bShowPopup = true;
        Interface::m_pPopupFunc = Interface::QuickObjectCreateMenu;
        Interface::m_popupTitle = "Quick object creator";

    }
}

static void ContextMenu_SnapToGround()
{
    CVector objPos = ObjManager::m_pSelected->GetPosition();
    int hObj = CPools::GetObjectRef(ObjManager::m_pSelected);
    float offZ = objPos.z - ObjManager::GetBoundingBoxGroundZ(ObjManager::m_pSelected);
    objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
    Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos.x, objPos.y, objPos.z);
}

static void ContextMenu_Copy()
{
    if (Viewport::m_HoveredEntity)
    {
        Viewport::COPY_MODEL::m_nModel = Viewport::m_HoveredEntity->m_nModelIndex;

        CVector &rot = Viewport::COPY_MODEL::m_vecRot;
        // Store rotation
        CallMethod<0x59A840, int>((int)Viewport::m_HoveredEntity->GetMatrix(),
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

static void ContextMenu_Paste()
{
    if (!Viewport::COPY_MODEL::m_nModel)
    {
        return;
    }

    CEntity *pEntity;
    CVector pos;
    if (Command<Commands::IS_MODEL_AVAILABLE>(Viewport::COPY_MODEL::m_nModel)
            && Utils::TraceEntity(pEntity, pos))
    {
        int hObj;
        Command<Commands::REQUEST_MODEL>(Viewport::COPY_MODEL::m_nModel);
        Command<Commands::LOAD_ALL_MODELS_NOW>();
        Command<Commands::CREATE_OBJECT>(Viewport::COPY_MODEL::m_nModel, pos.x, pos.y, pos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(Viewport::COPY_MODEL::m_nModel);

        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = ObjManager::m_objData.Get(pEntity);
        data.m_modelName = ObjManager::FindNameFromModel(Viewport::COPY_MODEL::m_nModel);

        if (ObjManager::bRandomRot)
        {
            Viewport::COPY_MODEL::m_vecRot.x = Random(ObjManager::randomRotX[0], ObjManager::randomRotX[1]);
            Viewport::COPY_MODEL::m_vecRot.y = Random(ObjManager::randomRotY[0], ObjManager::randomRotY[1]);
            Viewport::COPY_MODEL::m_vecRot.z = Random(ObjManager::randomRotZ[0], ObjManager::randomRotZ[1]);
        }
        
        data.SetRotation(Viewport::COPY_MODEL::m_vecRot);

        ObjManager::m_pVecEntities.push_back(pEntity);
        ObjManager::m_pSelected = pEntity;
    }
}

static void ContextMenu_Delete()
{
    if (ObjManager::m_pSelected)
    {
        ObjManager::m_pSelected->Remove();
        ObjManager::m_pVecEntities.erase(std::remove(ObjManager::m_pVecEntities.begin(),
                                         ObjManager::m_pVecEntities.end(), ObjManager::m_pSelected), ObjManager::m_pVecEntities.end());

        ObjManager::m_pSelected = nullptr;
    }
}

void Viewport::ProcessContextMenu()
{
    if (Interface::Browser::m_bShown || m_eViewportMode != EDIT_MODE)
    {
        return;
    }

    if (ImGui::BeginPopupContextWindow("ContextMenu"))
    {
        if (ImGui::MenuItem("New object"))
        {
            ContextMenu_NewObject();
            m_bShowContextMenu = false;
        }
        if (ImGui::MenuItem("Add to favourites"))
        {
            int model = Viewport::m_HoveredEntity->m_nModelIndex;
            std::string keyName = std::to_string(model) + " - " + ObjManager::FindNameFromModel(model);
            Interface::m_favData.m_pJson->m_Data["All"][keyName] = std::to_string(model);
            Interface::m_favData.m_pJson->WriteToDisk();
            CHud::SetHelpMessage("Added to favourites", false, false, false);
            m_bShowContextMenu = false;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Snap to ground"))
        {
            ContextMenu_SnapToGround();
            m_bShowContextMenu = false;
        }

        if (ImGui::MenuItem("Copy"))
        {
            ContextMenu_Copy();
            m_bShowContextMenu = false;
        }

        if (ImGui::MenuItem("Paste"))
        {
            ContextMenu_Paste();
            m_bShowContextMenu = false;
        }

        if (ImGui::MenuItem("Delete"))
        {
            ContextMenu_Delete();
            m_bShowContextMenu = false;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close"))
        {
            m_bShowContextMenu = false;
        }
        ImGui::EndPopup();
    }
}

void Viewport::ProcessSelectedObjectInputs()
{
    if (!m_bBeingHovered)
    {
        return;
    }

    // -------------------------------------------------
    // get object selection
    if (ImGui::IsMouseDoubleClicked(0))
    {
        CEntity *pEntity;
        CVector pos;

        if (Utils::TraceEntity(pEntity, pos))
        {
            ObjManager::m_pSelected = nullptr;
            for (auto &ent : ObjManager::m_pVecEntities)
            {
                if (ent == pEntity)
                {
                    ObjManager::m_pSelected = (CObject*)pEntity;
                    break;
                }
            }
        }
    }

    // -------------------------------------------------
    // Process object movement
    static int hObj = NULL;
    static CVector off; // offset between the mouse and the object pivot

    if (!(Interface::m_bShowPopup && m_bShowContextMenu))
    {
        // -------------------------------------------------
        // X, Y, Z axis movement
        // TODO: Z axis is kinda buggy

        if (!Interface::Browser::m_bShown)
        {
            static bool bObjectBeingDragged;

            if (ImGui::IsMouseDown(0) && ObjManager::m_pSelected)
            {
                CEntity *pEntity;
                static CVector pos, off;
                bool bFound = Utils::TraceEntity(pEntity, pos);

                if (bFound)
                {
                    if (bObjectBeingDragged)
                    {
                        auto &data = ObjManager::m_objData.Get(ObjManager::m_pSelected);
                        CVector objPos = CVector(pos.x - off.x, pos.y - off.y, pos.z - off.z);

                        if (Interface::m_bAutoSnapToGround)
                        {
                            float offZ = objPos.z - ObjManager::GetBoundingBoxGroundZ(ObjManager::m_pSelected);
                            objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
                            off.z = pos.z - objPos.z;
                        }

                        Command<Commands::SET_OBJECT_COORDINATES>(data.handle, objPos.x, objPos.y, objPos.z);
                    }
                    else
                    {
                        if (pEntity == ObjManager::m_pSelected)
                        {
                            off = pos - pEntity->GetPosition();
                            bObjectBeingDragged = true;
                        }
                    }
                }
            }
            else
            {
                if (bObjectBeingDragged
                        && Interface::m_bAutoSnapToGround && ObjManager::m_pSelected)
                {
                    auto &data = ObjManager::m_objData.Get(ObjManager::m_pSelected);
                    CVector pos = ObjManager::m_pSelected->GetPosition();
                    float offZ = pos.z - ObjManager::GetBoundingBoxGroundZ(ObjManager::m_pSelected);
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
        if (wheel && ObjManager::m_pSelected && Viewport::m_eViewportMode == EDIT_MODE)
        {
            if (KeyPressed(VK_LCONTROL))
            {
                auto &data = ObjManager::m_objData.Get(ObjManager::m_pSelected);
                CVector rot = data.GetRotation();
                rot.z += 3*wheel;

                if (rot.z > 360.0f)
                {
                    rot.z -= 360.0f;
                }

                if (rot.z < 0.0f)
                {
                    rot.z += 360.0f;
                }

                data.SetRotation(rot);
            }
            else
            {
                CVector objPos = ObjManager::m_pSelected->GetPosition();
                Command<Commands::SET_OBJECT_COORDINATES>(CPools::GetObjectRef(ObjManager::m_pSelected),
                        objPos.x, objPos.y, objPos.z + wheel/3);
            }
        }


        if (!Interface::m_bIsInputLocked)
        {
            // -------------------------------------------------
            // Context menu shortcuts
            if (newObjKey.Pressed())
            {
                ContextMenu_NewObject();
            }
            if (copyKey.Pressed())
            {
                ContextMenu_Copy();
            }
            if(pasteKey.Pressed())
            {
                ContextMenu_Paste();
            }
            if(deleteKey.Pressed())
            {
                ContextMenu_Delete();
            }
            if(snapKey.Pressed())
            {
                ContextMenu_SnapToGround();
            }
            // -------------------------------------------------
        }
    }
    else
    {
        hObj = NULL;
    }
    // -------------------------------------------------
}