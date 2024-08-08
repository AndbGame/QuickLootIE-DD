#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <wrl/client.h>

#include "InterfaceDEC.h"

namespace QuickLootDD
{
	struct UIItemInfoData
	{
		double itemChance = 0.0;
		std::size_t count = 0;
		double totalChance = 0.0;
	};
	static const std::size_t UIItemInfoDataLength = 5;
	struct UIInfoData
	{
		bool isCooldown = false;
		double containerChance = 0.0;
		double locationChance = 0.0;
		double limitChance = 0.0;
		float visualiseMinIntensity = 0.0f;
		float visualiseMaxIntensity = 0.0f;
		float visualiseColorR = 0.0f;
		float visualiseColorG = 0.0f;
		float visualiseColorB = 0.0f;
		float visualiseColorAmin = 0.0f;
		float visualiseColorAmax = 0.0f;
		UIItemInfoData itemChance[UIItemInfoDataLength];
		std::size_t itemCount = 0;
		bool verbose = false;
	};

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
		static inline std::atomic<UIInfoData> currentLootInfo = {};

		static bool isNeedRenderFrame();
		static void SetWindowDimensions(float a_offsetX = 0.f, float a_offsetY = 0.f, float a_sizeX = -1.f, float a_sizeY = -1.f, WindowAlignment a_alignment = WindowAlignment::kTopLeft, ImVec2 a_sizeMin = ImVec2(0, 0), ImVec2 a_sizeMax = ImVec2(0, 0), ImGuiCond_ a_cond = ImGuiCond_FirstUseEver);

        static bool LoadTextureFromFile();
        

	private:
		UI() = delete;

		static inline std::atomic<bool> ShowUI = false;
		static inline std::atomic<bool> ResetOverlay = false;
		static inline double overlayTimer = 0;
		static inline double timerTicks = 0;
		static inline REX::W32::ID3D11Device* device = nullptr;
		static inline REX::W32::ID3D11DeviceContext* context = nullptr;

		static inline Microsoft::WRL::ComPtr<REX::W32::ID3D11ShaderResourceView> srView{ nullptr };
		static inline float x;
		static inline float y;

	public:

		static bool Install();
		static void OnDataLoaded();

        static void Close();
		static void ShowDECInfo(UIInfoData data);
	};
}