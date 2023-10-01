#include "pch.h"
#include "fontmgr.h"

ImFont* FontMgr::Get(const char* fontID) {
    for (auto &data : m_vecFonts) {
        if (!strcmp(data.m_ID.c_str(), fontID)) {
            return data.m_pFont;
        }
    }

    return nullptr;
}

const ImWchar* FontMgr::GetGlyphRanges() {
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0,
    };
    return &ranges[0];
}

ImFont* FontMgr::LoadFont(const char* fontID, const unsigned int* func, unsigned int size, float fontMul) {
    ImFont* font;
    ImGuiIO& io = ImGui::GetIO();
    size_t fontSize = static_cast<int>(screen::GetScreenHeight() / 54.85f) * fontMul;

    font = io.Fonts->AddFontFromMemoryCompressedTTF(func, size, fontSize, NULL, GetGlyphRanges());
    m_vecFonts.push_back({font, size, fontMul, std::string(fontID), func});
    io.Fonts->Build();

    return font;
}

void FontMgr::UnloadAll() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->ClearFonts();
}

void FontMgr::ReloadAll() {
    UnloadAll();

    ImGuiIO& io = ImGui::GetIO();
    for (auto &data : m_vecFonts) {
        size_t fontSize = static_cast<int>(screen::GetScreenHeight() / 54.85f) * data.m_fMul;
        data.m_pFont = io.Fonts->AddFontFromMemoryCompressedTTF(data.m_pfunc, data.m_nSize, fontSize, NULL, GetGlyphRanges());
    }
    io.FontDefault = Get("text");
    m_bFontReloadRequired = false;
}

bool FontMgr::IsFontReloadRequired() {
    return m_bFontReloadRequired;
}

void FontMgr::StartOptionalFontDownload() {
    if (curState == eStates::Idle) {
        curState = eStates::Downloading;
    }
}

void FontMgr::SetFontReloadRequired(bool state) {
    m_bFontReloadRequired = state;
}