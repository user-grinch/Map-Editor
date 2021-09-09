#include "pch.h"
#include "Hook.h"

LRESULT Hook::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (ImGui::GetIO().WantTextInput)
	{
		Call<0x53F1E0>(); // CPad::ClearKeyboardHistory
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT Hook::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	return oReset(pDevice, pPresentationParameters);
}

HRESULT CALLBACK Hook::RenderFrame(IDirect3DDevice9* pDevice)
{
	if (!ImGui::GetCurrentContext())
	{
		return oEndScene(pDevice);;
	}

	ImGuiIO& io = ImGui::GetIO();
	static bool bInit;
	if (bInit)
	{
		ShowMouse(m_bShowMouse);

		// handle window scaling here
		ImVec2 size(screen::GetScreenWidth(), screen::GetScreenHeight());
		static ImVec2 fScreenSize;
		if (fScreenSize.x != size.x && fScreenSize.y != size.y)
		{
			int fontSize = static_cast<int>(size.y / 54.85f); // manually tested

			io.FontDefault = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/trebucbd.ttf", fontSize);
			io.Fonts->Build();

			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGuiStyle* style = &ImGui::GetStyle();
			float scaleX = size.x / 1366.0f;
			float scaleY = size.y / 768.0f;

			style->FramePadding = ImVec2(5 * scaleX, 3 * scaleY);
			style->ItemSpacing = ImVec2(8 * scaleX, 4 * scaleY);
			style->ScrollbarSize = 12 * scaleX;
			style->IndentSpacing = 20 * scaleX;
			style->ItemInnerSpacing = ImVec2(4 * scaleX, 4 * scaleY);

			fScreenSize = size;
		}

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX9_NewFrame();

		ImGui::NewFrame();

		if (windowCallback != nullptr)
		{
			windowCallback();
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	else
	{
		bInit = true;
		ImGui::CreateContext();

		ImGuiStyle& style = ImGui::GetStyle();

		ImGui_ImplWin32_Init(RsGlobal.ps->window);

		// shift trigger fix
		patch::Nop(0x00531155, 5);

		ImGui_ImplDX9_Init(GetD3DDevice());
		ImGui_ImplWin32_EnableDpiAwareness();

		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		style.WindowTitleAlign = ImVec2(0.5, 0.5);
		oWndProc = (WNDPROC)SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)WndProc);
	}

	return oEndScene(pDevice);
}
void Hook::ShowMouse(bool state)
{
	if (m_bMouseVisibility != m_bShowMouse)
	{
		ImGui::GetIO().MouseDrawCursor = state;

		Hook::ApplyMouseFix(); // Reapply the patches

		CPad::NewMouseControllerState.X = 0;
		CPad::NewMouseControllerState.Y = 0;
		CPad::ClearMouseHistory();
		CPad::UpdatePads();
		m_bMouseVisibility = m_bShowMouse;
	}
}

PBYTE HookVTableFunction(PDWORD* dwVTable, PBYTE dwHook, INT Index)
{
    DWORD dwOld = 0;
    VirtualProtect((void*)((*dwVTable) + (Index*4) ), 4, PAGE_EXECUTE_READWRITE, &dwOld);
    PBYTE pOrig = ((PBYTE)(*dwVTable)[Index]);
    (*dwVTable)[Index] = (DWORD)dwHook;
    VirtualProtect((void*)((*dwVTable) + (Index*4)), 4, dwOld, &dwOld);
    return pOrig;
}

Hook::Hook()
{
	ImGui::CreateContext();
	oReset = (f_Reset)HookVTableFunction((PDWORD*)GetD3DDevice(), (PBYTE)Reset, 16);
	oEndScene = (f_EndScene)HookVTableFunction((PDWORD*)GetD3DDevice(), (PBYTE)RenderFrame, 42);
}

Hook::~Hook()
{
	SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)oWndProc);
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

struct Mouse
{
	unsigned int x, y;
	unsigned int wheelDelta;
	char k1, k2, k3, k4, k5;
};

struct MouseInfo
{
	int x, y, wheelDelta;
} mouseInfo;

static BOOL __stdcall _SetCursorPos(int X, int Y)
{
	if (Hook::m_bShowMouse || GetActiveWindow() != RsGlobal.ps->window)
		return 1;

	mouseInfo.x = X;
	mouseInfo.y = Y;

	return SetCursorPos(X, Y);
}

static LRESULT __stdcall _DispatchMessage(MSG* lpMsg)
{
	if (lpMsg->message == WM_MOUSEWHEEL && !Hook::m_bShowMouse)
	{
		mouseInfo.wheelDelta += GET_WHEEL_DELTA_WPARAM(lpMsg->wParam);
	}

	return DispatchMessageA(lpMsg);
}

static int _cdecl _GetMouseState(Mouse* pMouse)
{
	if (Hook::m_bShowMouse)
	{
		return -1;
	}

	struct tagPOINT Point;

	pMouse->x = 0;
	pMouse->y = 0;
	pMouse->wheelDelta = mouseInfo.wheelDelta;
	GetCursorPos(&Point);

	if (mouseInfo.x >= 0)
	{
		pMouse->x = int(Point.x - mouseInfo.x);
	}

	if (mouseInfo.y >= 0)
	{
		pMouse->y = int(Point.y - mouseInfo.y);
	}

	mouseInfo.wheelDelta = 0;

	pMouse->k1 = (GetAsyncKeyState(1) >> 8);
	pMouse->k2 = (GetAsyncKeyState(2) >> 8);
	pMouse->k3 = (GetAsyncKeyState(4) >> 8);
	pMouse->k4 = (GetAsyncKeyState(5) >> 8);
	pMouse->k5 = (GetAsyncKeyState(6) >> 8);
	return 0;
}

void Hook::ApplyMouseFix()
{
	patch::ReplaceFunctionCall(0x53F417, _GetMouseState);
	patch::Nop(0x57C59B, 1);
	patch::ReplaceFunctionCall(0x57C59C, _SetCursorPos);
	patch::Nop(0x81E5D4, 1);
	patch::ReplaceFunctionCall(0x81E5D5, _SetCursorPos);
	patch::Nop(0x74542D, 1);
	patch::ReplaceFunctionCall(0x74542E, _SetCursorPos);
	patch::Nop(0x748A7C, 1);
	patch::ReplaceFunctionCall(0x748A7D, _DispatchMessage);
}