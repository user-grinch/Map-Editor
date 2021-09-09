#pragma once
#include "pch.h"

using f_EndScene = HRESULT(CALLBACK*)(IDirect3DDevice9*);
using f_Reset = HRESULT(CALLBACK*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Hook
{
private:
	static inline WNDPROC oWndProc;
	static inline f_EndScene oEndScene;
	static inline f_Reset oReset;
	static inline bool m_bMouseVisibility; // internal mouse state flag

	static HRESULT CALLBACK RenderFrame(IDirect3DDevice9* pDevice);
	static HRESULT CALLBACK Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void ShowMouse(bool state);

public:
	static inline bool m_bShowMouse = false; // external mouse state flag
	static inline std::function<void()> windowCallback = nullptr; // callback to draw window code
	
	Hook();
	~Hook();

	static void ApplyMouseFix();
};
