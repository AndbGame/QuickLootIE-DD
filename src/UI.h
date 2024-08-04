#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "InterfaceDEC.h"

namespace QuickLootDD
{
	class UI
	{
		struct WndProcHook
		{
			static LRESULT thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			static inline WNDPROC func;
		};

		struct D3DInitHook
		{
			static void thunk();
			static inline REL::Relocation<decltype(thunk)> func;

			static constexpr auto id = REL::RelocationID(75595, 77226);
			static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00);  // VR unknown

			static inline std::atomic<bool> initialized = false;
		};

		class MenuPresentHook : public RE::HUDMenu
		{
		public:
			static void Install()
			{
				REL::Relocation<std::uintptr_t> vtbl{ VTABLE[0] };
				func = vtbl.write_vfunc(0x6, &Hook_PostDisplay);
			}

		private:
			static void Hook_PostDisplay(RE::IMenu* Menu);
			static inline REL::Relocation<decltype(&PostDisplay)> func;
		};

		enum class WindowAlignment : uint8_t
		{
			kTopLeft = 0,
			kTopCenter,
			kTopRight,
			kCenterLeft,
			kCenter,
			kCenterRight,
			kBottomLeft,
			kBottomCenter,
			kBottomRight,
		};

		struct WindowSizeData
		{
			ImVec2 sizeMin;
			ImVec2 sizeMax;
			ImVec2 pos;
			ImVec2 pivot;
			ImVec2 size;
		};

        static void renderFrame();
		static void renderUI();

        static inline std::atomic<bool> ShowLootInfo = false;
		static inline std::atomic<InterfaceDeviouslyEnchantedChests::UIInfoData> currentLootInfo = {};

		static bool isNeedRenderFrame();
		static void SetWindowDimensions(float a_offsetX = 0.f, float a_offsetY = 0.f, float a_sizeX = -1.f, float a_sizeY = -1.f, WindowAlignment a_alignment = WindowAlignment::kTopLeft, ImVec2 a_sizeMin = ImVec2(0, 0), ImVec2 a_sizeMax = ImVec2(0, 0), ImGuiCond_ a_cond = ImGuiCond_FirstUseEver);
        

	private:
		UI() = delete;

		static inline std::atomic<bool> ShowUI = false;
		static inline REX::W32::ID3D11Device* device = nullptr;
		static inline REX::W32::ID3D11DeviceContext* context = nullptr;

	public:

		static bool Install();
		static void OnDataLoaded();

        static void Close();
		static void ShowDECInfo(InterfaceDeviouslyEnchantedChests::UIInfoData data);
	};
}