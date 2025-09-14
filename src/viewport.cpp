#include "pch.h"
#include "viewport.h"
#include "utils/utils.h"
#include "entitymgr.h"
#include "interface.h"
#include "utils/hotkeys.h"
#include <CHud.h>
#include <CModelInfo.h>
#include <CStreaming.h>
#include <CTxdStore.h>
#include <CSprite.h>
#include <D3dx9math.h>
#include "editor.h"

#include "contextmenus.h"
#include "popups.h"
#include "tooltips.h"

// Backups for restoring later on editor exit
static BYTE m_bHudState;
static BYTE m_bRadarState;
static bool m_bNeverWanted;

ImVec2 ViewportMgr::GetSize() {
    return m_fViewportSize;
}

ViewportMgr Viewport;
ViewportMgr::ViewportMgr() {
    Events::initGameEvent += [this]() {
        Viewport.m_nMoveSpeedMul = gConfig.Get("editor.moveSpeed", 1.0f);
    };

    // Highlight selection
    injector::MakeInline(0x534388, 0x53438E, [](injector::reg_pack& regs) {
        regs.edx |= 0x2000;
        Viewport.HighlightSelection((CEntity*)regs.esi);
    });
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
    pPlayer->bIsVisible = true;

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

    m_Renderer.Init();
    m_bInitialized = true;
}

/*
*  Part of the source is taken from DrawColsSA by Sergeanur
*  https://github.com/Sergeanur/
*/

static void RenderLineWithClipping(float x1, float y1, float z1, float x2, float y2, float z2, unsigned int c1, unsigned int c2) {
    ((void (__cdecl *)(float, float, float, float, float, float, unsigned int, unsigned int))0x6FF4F0)(x1, y1, z1, x2, y2, z2, c1, c2);
}

static void NodeWrapperRecursive(RwFrame* frame, CEntity* pEntity, std::function<void(RwFrame*)> func) {
    if (frame) {
        func(frame);
        if (RwFrame* newFrame = frame->child) {
            NodeWrapperRecursive(newFrame, pEntity, func);
        }
        if (RwFrame* newFrame = frame->next) {
            NodeWrapperRecursive(newFrame, pEntity, func);
        }
    }
    return;
}

void ViewportMgr::HighlightSelection(CEntity *pEntity) {
    static CEntity *prevEntity = nullptr;
    if (!Editor.IsOpen() || !pEntity || !pEntity->m_pRwClump) {
        return;
    }

    if (prevEntity && prevEntity->m_nModelIndex != pEntity->m_nModelIndex 
    && EntMgr.m_pSelected && pEntity->m_nModelIndex != EntMgr.m_pSelected->m_nModelIndex) {
        return; 
    }

    if (pEntity->m_nType == ENTITY_TYPE_BUILDING || pEntity->m_nType == ENTITY_TYPE_OBJECT) {
        NodeWrapperRecursive((RwFrame*)pEntity->m_pRwClump->object.parent, pEntity, [&](RwFrame* frame) {
            RwFrameForAllObjects(frame, [](RwObject* object, void* data) -> RwObject* {
                if (object->type == rpATOMIC) {
                    RpAtomic* atomic = reinterpret_cast<RpAtomic*>(object);
                    CEntity* pEntity = reinterpret_cast<CEntity*>(data);
                    for (int i = 0; i < atomic->geometry->matList.numMaterials; ++i) {
                        if (pEntity == EntMgr.m_pSelected && !Viewport.m_Renderer.m_bShown) {
                            atomic->geometry->matList.materials[i]->color = {255, 0, 0, 255}; 
                            atomic->geometry->flags |= rpGEOMETRYMODULATEMATERIALCOLOR;
                        } 
                        else {
                            atomic->geometry->matList.materials[i]->color = {255, 255, 255, 255};
                        }
                    }
                }
                return object;
            }, pEntity);
        });
    }
    prevEntity = pEntity;
}

void ViewportMgr::SetCameraPosn(const CVector &pos) {
    m_fCamPos = pos;

    /*
    * Since we're actually setting the player's position,
    * we need to add the offset to it
    */
    m_fCamPos.z -= PLAYER_Z_OFFSET;
    m_bCamUpdateRequired = true;
}

void ViewportMgr::DrawHoverMenu() {
    if ( m_eState != eViewportState::Edit || !m_bHovered || m_Renderer.m_bShown) {
        return;
    }

    static CVector worldPos;
    if (Utils::TraceEntity(m_HoveredEntity, worldPos) && Interface.m_bShowHoverMenu && !Viewport.m_Renderer.m_bShown) {
        Tooltip.m_Title = "##Hover Info";
        Tooltip.m_bShow = true;
        Tooltip.m_pFunc = [this]() {
            ImGui::Text("Hover Info");
            ImGui::Separator();
            int model = m_HoveredEntity->m_nModelIndex;
            if (m_HoveredEntity->m_nType == ENTITY_TYPE_OBJECT
                    || m_HoveredEntity->m_nType == ENTITY_TYPE_BUILDING) {
                static int bmodel = 0;
                static std::string name = "";

                // lets not go over 20000 models each frame
                if (bmodel != model) {
                    name = EntMgr.FindNameFromModel(model);
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
            ImGui::Text("Position: %d, %d, %d", int(worldPos.x), int(worldPos.y), int (worldPos.z));
        };
    }
    Tooltip.Draw();
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
    m_Renderer.m_bShown = false;
    m_Renderer.m_bShowNextFrame = false;
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
    m_fViewportSize = ImVec2(width-menuWidth, height - frameHeight);
    ImGui::SetNextWindowSize(m_fViewportSize);

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
        ContextMenu.Draw();
        ImGui::End();
    }
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
            ContextMenu.m_bShow = true;
            ContextMenu.m_pFunc = ContextMenu_Viewport;
            ContextMenu.m_Root = "";
            ContextMenu.m_Key = "";
            ContextMenu.m_Val = "";
        }
    }

    m_Renderer.Process();

    // -------------------------------------------------
    // vars
    int delta = (CTimer::m_snTimeInMillisecondsNonClipped -
                 CTimer::m_snPreviousTimeInMillisecondsNonClipped);

    int ratio = 1 / (1 + (delta * m_nMoveSpeedMul));
    float speed = (m_nMoveSpeedMul + m_nMoveSpeedMul * ratio * delta);

    CPlayerPed* pPlayer = FindPlayerPed(-1);
    int hPlayer = CPools::GetPedRef(pPlayer);
    CVector pos = pPlayer->GetPosition();
    // -------------------------------------------------
    // Check if camera position was updated externally
    // If it was then change to new values
    if (m_bCamUpdateRequired) {
        pos = m_fCamPos;
        m_bCamUpdateRequired = false;
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
    m_fMousePos.y = m_fMousePos.y > 150.0f ? 150.0f : m_fMousePos.y;
    m_fMousePos.y = m_fMousePos.y < -150.0f ? -150.0f : m_fMousePos.y;

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
    if (m_eState == eViewportState::View && !m_Renderer.m_bShown) {
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
            EntMgr.m_pSelected = nullptr;
            for (auto &ent : EntMgr.m_pPlaced) {
                if (ent == pEntity) {
                    EntMgr.m_pSelected = (CObject*)pEntity;
                    break;
                }
            }
        }
    }

    // -------------------------------------------------
    // Process object movement
    static int hObj = NULL;
    static CVector off; // offset between the mouse and the object pivot

    if (!(Popup.m_bShow && ContextMenu.m_bShow)) {
        // -------------------------------------------------
        // X, Y, Z axis movement
        // TODO: Z axis is kinda buggy

        if (!m_Renderer.m_bShown) {
            static bool bObjectBeingDragged;

            if (ImGui::IsMouseDown(0) && EntMgr.m_pSelected) {
                CEntity *pEntity;
                static CVector pos, off;
                bool bFound = Utils::TraceEntity(pEntity, pos);

                if (bFound) {
                    if (bObjectBeingDragged) {
                        auto &data = EntMgr.m_Info.Get(EntMgr.m_pSelected);
                        CVector objPos = CVector(pos.x - off.x, pos.y - off.y, pos.z - off.z);

                        if (Interface.m_bAutoSnapToGround) {
                            float offZ = objPos.z - EntMgr.GetBoundingBoxGroundZ(EntMgr.m_pSelected);
                            objPos.z = CWorld::FindGroundZFor3DCoord(objPos.x, objPos.y, objPos.z + 100.0f, nullptr, nullptr) + offZ;
                            off.z = pos.z - objPos.z;
                        }

                        Command<Commands::SET_OBJECT_COORDINATES>(data.m_nHandle, objPos.x, objPos.y, objPos.z);
                    } else {
                        if (pEntity == EntMgr.m_pSelected) {
                            off = pos - pEntity->GetPosition();
                            bObjectBeingDragged = true;
                        }
                    }
                }
            } else {
                if (bObjectBeingDragged
                        && Interface.m_bAutoSnapToGround && EntMgr.m_pSelected) {
                    auto &data = EntMgr.m_Info.Get(EntMgr.m_pSelected);
                    CVector pos = EntMgr.m_pSelected->GetPosition();
                    float offZ = pos.z - EntMgr.GetBoundingBoxGroundZ(EntMgr.m_pSelected);
                    pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 100.0f, nullptr, nullptr) + offZ;
                    Command<Commands::SET_OBJECT_COORDINATES>(data.m_nHandle, pos.x, pos.y, pos.z);
                }
                bObjectBeingDragged = false;
            }
        }

        // -------------------------------------------------
        // Z axis movement
        ImGuiIO &io = ImGui::GetIO();
        float wheel = io.MouseWheel;
        if (wheel && EntMgr.m_pSelected && m_eState == eViewportState::Edit) {
            if (KeyPressed(VK_LCONTROL)) {
                auto &data = EntMgr.m_Info.Get(EntMgr.m_pSelected);
                CVector rot = data.GetEuler();
                rot.z += 3*wheel;

                if (rot.z > 360.0f) {
                    rot.z -= 360.0f;
                }

                if (rot.z < 0.0f) {
                    rot.z += 360.0f;
                }

                data.SetEuler(rot);
            } else {
                CVector objPos = EntMgr.m_pSelected->GetPosition();
                Command<Commands::SET_OBJECT_COORDINATES>(CPools::GetObjectRef(EntMgr.m_pSelected),
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
                Action_RemoveSelectedObject();
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