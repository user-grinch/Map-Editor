#include "renderer.h"
#include "pch.h"

#include <CStreaming.h>
#include <CModelInfo.h>
#include <CScene.h>

#include "viewport.h"

void ModelRenderer::UnloadModel() {
    CBaseModelInfo *pInfo = (CBaseModelInfo *)CModelInfo::GetModelInfo(m_nRenderModel);
    if (pInfo) {
        pInfo->RemoveRef();
    }

    if (m_pRpClump) {
        RpClumpDestroy(m_pRpClump);
        m_pRpClump = nullptr;
    }

    if (m_pRpAtomic) {
        RpAtomicDestroy(m_pRpAtomic);
        m_pRpAtomic = nullptr;
    }

    if (m_pRwFrame) {
        RwFrameDestroy(m_pRwFrame);
        m_pRwFrame = nullptr;
    }
    Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(m_nRenderModel);

    // Reset
    m_fPos = { 0.f, 0.05f, 2.0f };
    m_fAxisX = { 1.0f, 0.0f, 0.0f };
    m_fAxisY = { 0.0f, 0.0f, 1.0f };
}

void ModelRenderer::RenderModel() {
    static uint32_t timer = NULL;
    if (m_bAutoRot && (CTimer::m_snTimeInMilliseconds - timer > 7)) {
        m_fRot.x += 1.0f;
        if (m_fRot.x > 360.0f)
            m_fRot.x -= 360.0f;
        timer = CTimer::m_snTimeInMilliseconds;
    }

    
    if (KeyPressed(VK_LBUTTON)) {
        float w = RsGlobal.maximumWidth;
        float h = RsGlobal.maximumHeight;
        static POINT prevPos = {-1, -1};
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        
        float menu_pos_x = w - w/5 - 5.0f; // TODO FIX
        if (cursorPos.x < menu_pos_x && (prevPos.x != cursorPos.x || prevPos.y != cursorPos.y)) {
            m_fPos.x = -1 * (cursorPos.x - w/2) / 400 * CTimer::ms_fTimeStep;
            m_fPos.y = -1 * (cursorPos.y - h/2) / 400 * CTimer::ms_fTimeStep;
            prevPos = cursorPos;
        }
    }

    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)true);

    RwFrameTransform(m_pRwFrame, &GetObjectParent(&Scene.m_pCamera->object.object)->modelling, rwCOMBINEREPLACE);
    RwFrameTranslate(m_pRwFrame, &m_fPos, rwCOMBINEPRECONCAT);
    RwV3d size = {m_fScale * 0.1f, m_fScale * 0.1f, m_fScale * 0.1f};
    RwFrameScale(m_pRwFrame, &size, rwCOMBINEPRECONCAT);
    RwFrameRotate(m_pRwFrame, &m_fAxisX, -90.0f + m_fRot.y, rwCOMBINEPRECONCAT);
    RwFrameRotate(m_pRwFrame, &m_fAxisY, m_fRot.x, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(m_pRwFrame);
    RwRGBAReal color = { 1.0f, 1.0f, 1.0f, 1.0f };
    SetAmbientColours(&color);

    if (m_pRpClump) {
        RpClumpRender(m_pRpClump);
    } else if (m_pRpAtomic) {
        m_pRpAtomic->renderCallBack(m_pRpAtomic);
    }
}

bool ModelRenderer::LoadModel() {
    CStreaming::RequestModel(m_nSelectedModel, PRIORITY_REQUEST);
    CStreaming::LoadAllRequestedModels(true);

    CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(m_nSelectedModel);
    eRwModelType type = static_cast<eRwModelType>(pInfo->GetRwModelType());
    if (type == eRwModelType::RpAtomic) {
        RpAtomic *atomic = reinterpret_cast<RpAtomic*>(pInfo->m_pRwObject);
        if (atomic) {
            pInfo->AddRef();

            m_pRwFrame = RwFrameCreate();
            m_pRpAtomic = RpAtomicClone(reinterpret_cast<RpAtomic*>(pInfo->m_pRwObject));
            RpAtomicSetFrame(m_pRpAtomic, m_pRwFrame);
            m_pRwFrame = static_cast<RwFrame*>(m_pRpAtomic->object.object.parent);
            if (m_pRpAtomic && m_pRwFrame) {
                return true;
            }
        }
    } else if (type == eRwModelType::RpClump) {
        m_pRpClump = reinterpret_cast<RpClump*>(pInfo->m_pRwObject);
        if (m_pRpClump) {
            pInfo->AddRef();
            m_pRpClump = reinterpret_cast<RpClump*>(reinterpret_cast<CClumpModelInfo*>(pInfo)->CreateInstance());
            m_pRwFrame = static_cast<RwFrame*>(m_pRpClump->object.parent);
            if (m_pRpClump && m_pRwFrame) {
                return true;
            }
        }
    }
    return false;
}


size_t ModelRenderer::GetSelected() {
    return m_nSelectedModel;
}

void ModelRenderer::SetSelected(int modelId) {
    if (modelId > 0 && Command<Commands::IS_MODEL_IN_CDIMAGE>(modelId)) {
        m_nSelectedModel = modelId;
    } else {
        m_nSelectedModel = NULL;
    }
}

void ModelRenderer::Init() {
    Events::drawingEvent += [this]() {
        if (m_bShown) {
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
            CSprite2d::DrawRect(pos, CRGBA(56, 56, 56, 255));
            // -------------------------------------------------

            if (m_nSelectedModel) {
                if (m_nRenderModel != m_nSelectedModel) {
                    UnloadModel();
                    if (LoadModel()) {
                        m_nRenderModel = m_nSelectedModel;
                    } else {
                        m_nRenderModel = NULL;
                    }
                }
            }

            if (m_nRenderModel) {
                RenderModel();
            }
        }
    };
}

void ModelRenderer::Process() {
    if (m_bShown && Viewport.m_eState == eViewportState::View) {
        CVector delta;
        Command<Commands::GET_PC_MOUSE_MOVEMENT>(&delta.x, &delta.y);
        m_fRot.y += delta.y / MOUSE_FACTOR_Y;

        if (!m_bAutoRot) {
            m_fRot.x += delta.x / MOUSE_FACTOR_X;
        }

        if (CPad::NewMouseControllerState.wheelUp) {
            m_fScale += 0.1f * CTimer::ms_fTimeStep;
            m_fScale = m_fScale > 5.0f ? 5.0f : m_fScale;
        }

        if (CPad::NewMouseControllerState.wheelDown) {
            m_fScale -= 0.1f * CTimer::ms_fTimeStep;
            m_fScale = m_fScale < 0.0f ? 0.0f : m_fScale;
        }
    }
}