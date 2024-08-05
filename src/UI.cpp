#include "UI.h"

#include <d3d11.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <dxgi.h>

#include "imgui_internal.h"

namespace QuickLootDD
{
	void UI::renderUI()
	{
        auto lootInfo = currentLootInfo.load();
		SetWindowDimensions(0, 0, 0, 0, WindowAlignment::kTopRight);
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin("QuickLootIEDD Debug", nullptr, windowFlags)) {
            if (lootInfo.isCooldown) {
				ImGui::Text("Container in Cooldown");
            } else {
				ImGui::Text("Container Chance: %f", lootInfo.containerChance);
				ImGui::Text("Location Chance: %f", lootInfo.locationChance);

				ImGui::SeparatorText("Item Info: ");
				for (std::size_t cnt = 0; cnt < lootInfo.itemCount; ++cnt) {
					ImGui::Text("Item Chance: %f", lootInfo.itemChance[cnt].itemChance);
					ImGui::Text("Total Chance: %f", lootInfo.itemChance[cnt].totalChance);
					if ((cnt + 1) < lootInfo.itemCount) {
						ImGui::Text("");
					}
				}
			}
		}
		ImGui::End();

	}
	void UI::SetWindowDimensions(float a_offsetX /*= 0.f*/, float a_offsetY /*= 0.f*/, float a_sizeX /*= -1.f*/, float a_sizeY /*= -1.f*/, WindowAlignment a_alignment /*= WindowAlignment::kTopLeft*/, ImVec2 a_sizeMin /*= ImVec2(0, 0)*/, ImVec2 a_sizeMax /*= ImVec2(0, 0)*/, ImGuiCond_ a_cond /*= ImGuiCond_FirstUseEver*/)
	{
		auto& io = ImGui::GetIO();

        WindowSizeData _sizeData;

		_sizeData.sizeMin = {
			std::fmin(300.f, io.DisplaySize.x - 40.f),
			std::fmin(200.f, io.DisplaySize.y - 40.f)
		};

		if (a_sizeMin.x != 0) {
			_sizeData.sizeMin.x = a_sizeMin.x;
		}
		if (a_sizeMin.y != 0) {
			_sizeData.sizeMin.y = a_sizeMin.y;
		}

		_sizeData.sizeMax = {
			io.DisplaySize.x,
			std::fmax(io.DisplaySize.y - 40.f, _sizeData.sizeMin.y)
		};

		if (a_sizeMax.x != 0) {
			_sizeData.sizeMax.x = a_sizeMax.x;
		}
		if (a_sizeMax.y != 0) {
			_sizeData.sizeMax.y = a_sizeMax.y;
		}

		switch (a_alignment) {
		case WindowAlignment::kTopLeft:
			_sizeData.pivot = { 0.f, 0.f };
			_sizeData.pos = {
				std::fmin(20.f + a_offsetX, io.DisplaySize.x - 40.f),
				20.f + a_offsetY
			};
			break;
		case WindowAlignment::kTopCenter:
			_sizeData.pivot = { 0.5f, 0.f };
			_sizeData.pos = {
				io.DisplaySize.x * 0.5f + a_offsetX,
				20.f + a_offsetY
			};
			break;
		case WindowAlignment::kTopRight:
			_sizeData.pivot = { 1.f, 0.f };
			_sizeData.pos = {
				std::fmax(io.DisplaySize.x - 20.f - a_offsetX, 40.f),
				20.f + a_offsetY
			};
			break;
		case WindowAlignment::kCenterLeft:
			_sizeData.pivot = { 0.f, 0.5f };
			_sizeData.pos = {
				20.f + a_offsetX,
				io.DisplaySize.y * 0.5f + a_offsetY
			};
			break;
		case WindowAlignment::kCenter:
			_sizeData.pivot = { 0.5f, 0.5f };
			_sizeData.pos = {
				io.DisplaySize.x * 0.5f + a_offsetX,
				io.DisplaySize.y * 0.5f + a_offsetY
			};
			break;
		case WindowAlignment::kCenterRight:
			_sizeData.pivot = { 1.f, 0.5f };
			_sizeData.pos = {
				std::fmax(io.DisplaySize.x - 20.f - a_offsetX, 40.f),
				io.DisplaySize.y * 0.5f + a_offsetY
			};
			break;
		case WindowAlignment::kBottomLeft:
			_sizeData.pivot = { 0.f, 1.f };
			_sizeData.pos = {
				std::fmin(20.f + a_offsetX, io.DisplaySize.x - 40.f),
				io.DisplaySize.y - 20.f - a_offsetY
			};
			break;
		case WindowAlignment::kBottomCenter:
			_sizeData.pivot = { 0.5f, 1.f };
			_sizeData.pos = {
				io.DisplaySize.x * 0.5f + a_offsetX,
				io.DisplaySize.y - 20.f - a_offsetY
			};
			break;
		case WindowAlignment::kBottomRight:
			_sizeData.pivot = { 1.f, 1.f };
			_sizeData.pos = {
				std::fmax(io.DisplaySize.x - 20.f - a_offsetX, 40.f),
				io.DisplaySize.y - 20.f - a_offsetY
			};
			break;
		}

		_sizeData.size = {
			a_sizeX < 0.f ? io.DisplaySize.x : a_sizeX,
			a_sizeY < 0.f ? io.DisplaySize.y : a_sizeY
		};

		ImGui::SetNextWindowPos(_sizeData.pos, a_cond, _sizeData.pivot);
		if (a_sizeX != 0.f || a_sizeY != 0.f) {
			ImGui::SetNextWindowSize(_sizeData.size, a_cond);
			ImGui::SetNextWindowSizeConstraints(_sizeData.sizeMin, _sizeData.sizeMax);
		}
	}

	bool UI::isNeedRenderFrame()
	{
		if (!ShowUI.load() || !ShowLootInfo.load()) {
			return false;
		}

		auto UI = RE::UI::GetSingleton();
		if (!UI || UI->GameIsPaused() || !UI->IsCursorHiddenWhenTopmost() || !UI->IsShowingMenus() || !UI->GetMenu<RE::HUDMenu>()) {
			return false;
		}

		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraState = camera ? camera->currentState : nullptr;
		if (cameraState == nullptr || (cameraState->id != RE::CameraState::kFirstPerson && cameraState->id != RE::CameraState::kThirdPerson)) {
			return false;
		}
		return true;
	}

	LRESULT UI::WndProcHook::thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto& io = ImGui::GetIO();
		if (uMsg == WM_KILLFOCUS) {
			io.ClearInputCharacters();
			io.ClearInputKeys();
		}

		return func(hWnd, uMsg, wParam, lParam);
	}

	void UI::D3DInitHook::thunk()
	{
		func();

		TRACE("D3DInit Hooked!");
		auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
		if (!render_manager) {
			ERROR("Cannot find render manager. Initialization failed!");
			return;
		}

		TRACE("Getting swapchain...");
		auto swapchain = render_manager->GetCurrentRenderWindow()->swapChain;
		if (!swapchain) {
			ERROR("Cannot find swapchain. Initialization failed!");
			return;
		}

		TRACE("Getting swapchain desc...");
		REX::W32::DXGI_SWAP_CHAIN_DESC sd{};
		if (swapchain->GetDesc(std::addressof(sd)) < 0) {
			ERROR("IDXGISwapChain::GetDesc failed.");
			return;
		}

		auto render_data = render_manager->GetRuntimeData();

		device = render_data.forwarder;
		context = render_data.context;

		TRACE("Initializing ImGui...");
		ImGui::CreateContext();
		if (!ImGui_ImplWin32_Init((void*)(sd.outputWindow))) {
			ERROR("ImGui initialization failed (Win32)");
			return;
		}
		if (!ImGui_ImplDX11_Init((ID3D11Device*)(device), (ID3D11DeviceContext*)(context))) {
			ERROR("ImGui initialization failed (DX11)");
			return;
		}

		RECT rect = { 0, 0, 0, 0 };
		GetClientRect((HWND)(sd.outputWindow), &rect);
		ImGui::GetIO().DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

		LOG("ImGui initialized!");

		initialized.store(true);

		WndProcHook::func = reinterpret_cast<WNDPROC>(
			SetWindowLongPtrA(
				sd.outputWindow,
				GWLP_WNDPROC,
				reinterpret_cast<LONG_PTR>(WndProcHook::thunk))
        );

		if (!WndProcHook::func) {
			ERROR("SetWindowLongPtrA failed!");
		}
	}

	bool UI::Install()
	{
		SKSE::AllocTrampoline(14 * 2);
		auto& trampoline = SKSE::GetTrampoline();
		REL::Relocation<std::uintptr_t> hook{ D3DInitHook::id, D3DInitHook::offset };
		D3DInitHook::func = trampoline.write_call<5>(hook.address(), D3DInitHook::thunk);

		MenuPresentHook::Install();

		return true;
	}

	void UI::OnDataLoaded()
	{
		DEBUG("UI OnDataLoaded");
		ShowUI.store(true);
	}

	void UI::Close()
	{
		ShowLootInfo.store(false);
	}

	void UI::ShowDECInfo(UIInfoData data)
	{
		currentLootInfo.store(data);
		ShowLootInfo.store(true);
	}

	void UI::renderFrame()
	{
        if (!isNeedRenderFrame()) {
			return;
        }
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		renderUI();
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void UI::MenuPresentHook::Hook_PostDisplay(RE::IMenu* Menu)
	{

		if (D3DInitHook::initialized.load()) {
			renderFrame();
		}

		func(Menu);
	}
}