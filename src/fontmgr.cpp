#include "fontmgr.h"
#include "pch.h"

ImFont* FontMgr::GetFont(const char* fontName)
{
    for (auto &data : m_vecFonts)
    {
        if (data.m_path == std::string(fontName))
        {
            return data.m_pFont;
        }
    }

    return nullptr;
}

ImFont* FontMgr::LoadFont(const char* fontPath, float fontMul)
{
    ImGuiIO& io = ImGui::GetIO();
    size_t fontSize = static_cast<int>(screen::GetScreenHeight() / 54.85f) * fontMul;

    m_vecFonts.push_back({io.Fonts->AddFontFromFileTTF(fontPath, fontSize), fontMul,
                          std::string(fontPath)});
    io.Fonts->Build();

    return m_vecFonts.back().m_pFont;
}

void FontMgr::UnloadFonts()
{
    ImGui::GetIO().Fonts->Clear();
}

void FontMgr::ReloadFonts()
{
    UnloadFonts();

    ImGuiIO& io = ImGui::GetIO();
    for (auto &data : m_vecFonts)
    {
        size_t fontSize = static_cast<int>(screen::GetScreenHeight() / 54.85f) * data.m_fMul;
        data.m_pFont = io.Fonts->AddFontFromFileTTF(data.m_path.c_str(), fontSize);
    }
    io.FontDefault = GetFont("text");
    io.Fonts->Build();
    m_bMulChangedExternal = false;
}

void FontMgr::SetMultiplier(float fontMul)
{
    for (auto &data : m_vecFonts)
    {
        data.m_fMul = fontMul;
    }
    m_bMulChangedExternal = true;
}

bool FontMgr::IsReloadNeeded()
{
    return m_bMulChangedExternal;
}