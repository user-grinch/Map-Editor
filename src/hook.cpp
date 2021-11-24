#include "pch.h"
#include "hook.h"
#include "../depend/kiero/kiero.h"
#include "../depend/kiero/minhook/MinHook.h"
#include "../depend/imgui/imgui_impl_dx9.h"
#include "../depend/imgui/imgui_impl_dx11.h"
#include "../depend/imgui/imgui_impl_win32.h"
#include <dinput.h>

LRESULT Hook::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (ImGui::GetIO().WantTextInput)
	{
		Call<0x53F1E0>(); // CPad::ClearKeyboardHistory
		return 1;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT Hook::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	return oReset(pDevice, pPresentationParameters);
}

void Hook::RenderFrame(void* ptr)
{
	if (!ImGui::GetCurrentContext())
	{
		ImGui::CreateContext();
	}

	ImGuiIO& io = ImGui::GetIO();
	static bool bInit = false;

	if (bInit)
	{
		ShowMouse(m_bShowMouse);	

		// Scale the menu if game resolution changed
		static ImVec2 fScreenSize = ImVec2(-1, -1);
		ImVec2 size(screen::GetScreenWidth(), screen::GetScreenHeight());
		if (fScreenSize.x != size.x && fScreenSize.y != size.y)
		{
			int fontSize = static_cast<int>(size.y / 54.85f); // manually tested
			io.FontDefault = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/trebucbd.ttf", fontSize);
			io.Fonts->Build();

			if (gRenderer == Render_DirectX9)
			{
				ImGui_ImplDX9_InvalidateDeviceObjects();
			}
			else
			{
				ImGui_ImplDX11_InvalidateDeviceObjects();
			}

			ImGuiStyle* style = &ImGui::GetStyle();
			float scaleX = size.x / 1366.0f;
			float scaleY = size.y / 768.0f;

			style->FramePadding = ImVec2(5 * scaleX, 5 * scaleY);
			style->ItemSpacing = ImVec2(8 * scaleX, 4 * scaleY);
			style->ScrollbarSize = 12 * scaleX;
			style->IndentSpacing = 20 * scaleX;
			style->ItemInnerSpacing = ImVec2(5 * scaleX, 5 * scaleY);

			fScreenSize = size;
		}

		ImGui_ImplWin32_NewFrame();
		if (gRenderer == Render_DirectX9)
		{
			ImGui_ImplDX9_NewFrame();
		}
		else
		{
			ImGui_ImplDX11_NewFrame();
		}

		ImGui::NewFrame();

		if (pCallbackFunc != nullptr)
		{
			pCallbackFunc();
		}

		ImGui::EndFrame();
		ImGui::Render();

		if (gRenderer == Render_DirectX9)
		{
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}
		else
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}
	else
	{
		bInit = true;
		ImGui_ImplWin32_Init(RsGlobal.ps->window);

		// shift trigger fix
		patch::Nop(0x00531155, 5);
		if (gRenderer == Render_DirectX9)
		{
			ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(ptr));
		}
		else
		{
			// for dx11 device ptr is swapchain
			reinterpret_cast<IDXGISwapChain*>(ptr)->GetDevice(__uuidof(ID3D11Device), &ptr);
			ID3D11DeviceContext* context;
			reinterpret_cast<ID3D11Device*>(ptr)->GetImmediateContext(&context);

			ImGui_ImplDX11_Init(reinterpret_cast<ID3D11Device*>(ptr), context);
		}

		ImGui_ImplWin32_EnableDpiAwareness();
		
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		oWndProc = (WNDPROC)SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)WndProc);
	}
}

HRESULT Hook::Dx9Handler(IDirect3DDevice9* pDevice)
{
	RenderFrame(pDevice);
	return oEndScene(pDevice);
}

HRESULT Hook::Dx11Handler(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	RenderFrame(pSwapChain);
	return oPresent11(pSwapChain, SyncInterval, Flags);
}

void Hook::ShowMouse(bool state)
{
	// Disable player controls for controllers
	bool bMouseDisabled = false;
	bool isController = patch::Get<BYTE>(0xBA6818);

	if (isController && (state || bMouseDisabled))
	{

		CPlayerPed *player = FindPlayerPed();
		CPad *pad = player ? player->GetPadFromPlayer() : NULL;

		if (pad)
		{
			if (state)
			{
				bMouseDisabled = true;
				pad->DisablePlayerControls = true;
			}
			else
			{
				bMouseDisabled = false;
				pad->DisablePlayerControls = false;
			}
		}
	}

	if (m_bMouseVisibility != state)
	{
		ImGui::GetIO().MouseDrawCursor = state;

		if (state)
		{
			
			patch::SetUChar(0x6194A0, 0xC3); // psSetMousePos
			patch::Nop(0x541DD7, 5); // don't call CPad::UpdateMouse()
		}
		else
		{
			
			patch::SetUChar(0x6194A0, 0xE9);
			patch::SetRaw(0x541DD7, (char*)"\xE8\xE4\xD5\xFF\xFF", 5);
		}

		CPad::NewMouseControllerState.X = 0;
		CPad::NewMouseControllerState.Y = 0;
		CPad::ClearMouseHistory();
		CPad::UpdatePads();
		m_bMouseVisibility = state;
	}
}

Hook::Hook()
{

	// Nvidia Overlay crash fix
	if (init(kiero::RenderType::D3D9) == kiero::Status::Success)
	{
		gRenderer = Render_DirectX9;
		kiero::bind(16, (void**)&oReset, Reset);
		kiero::bind(42, (void**)&oEndScene, Dx9Handler);
	}
	else
	{
		// gtaRenderHook
		if (init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			gRenderer = Render_DirectX11;
			kiero::bind(8, (void**)&oPresent11, Dx11Handler);
		}
	}
}

Hook::~Hook()
{
	SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)oWndProc);
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	kiero::shutdown();
}