#pragma once

#include "PluginRequests/RequestClient.h"

/*
	Header File for QuickLoot integration
*/

namespace QuickLoot::Integrations
{
	namespace TakenHandler
	{
		struct Element
		{
			RE::TESForm* object;
			std::int32_t count;
		};
		struct TakenEvent
		{
			RE::Actor* actor;
			Element* elements;
			std::size_t elementsCount;
			RE::TESObjectREFR* container;
		};
		typedef void (*OnTakenHandler)(TakenEvent* evt);

		struct Request
		{
			OnTakenHandler handler;
		};
	};

	namespace SelectHandler
	{
		struct Element
		{
			RE::TESForm* object;
			std::int32_t count;
		};
		struct SelectEvent
		{
			RE::Actor* actor;
			Element* elements;
			std::size_t elementsCount;
			RE::TESObjectREFR* container;
		};
		typedef void (*OnSelectHandler)(SelectEvent* evt);

		struct Request
		{
			OnSelectHandler handler;
		};
	};

	namespace LootMenuHandler
	{
		struct Element
		{
			RE::TESForm* object;
			std::int32_t count;
			RE::TESObjectREFR* container;
		};
		enum Status
		{
			OPEN,
			CLOSE,
			INVALIDATE,
		};
		struct LootMenuEvent
		{
			Status status;
			RE::Actor* actor;
			RE::TESObjectREFR* container;
			Element* elements;
			std::size_t elementsCount;
		};
		typedef void (*OnLootMenuHandler)(LootMenuEvent* evt);

		struct Request
		{
			OnLootMenuHandler handler;
		};
	};

	class PluginServer;

	class QuickLootAPI
	{
		friend class PluginServer;

	private:
		QuickLootAPI() = default;
		~QuickLootAPI() = default;

		// For a client request to be processed, both of the following must be true:
		//
		// - client major ver == server major ver
		// - client minor var <= server minor ver
		//
		// Minor version changes must be backwards compatible, so any breaking changes
		// to the api must increase the major version. New handlers may be added by
		// new minor versions.
		static constexpr const char* SERVER_PLUGIN_NAME = "QuickLootIE";
		static constexpr uint16_t API_MAJOR_VERSION = 1;
		static constexpr uint16_t API_MINOR_VERSION = 1;

		static inline PluginRequests::RequestClient _client{};

		// This is the list of request types.
		// Each of them is associated with a signature.
		// The client and server must agree on these signatures.
		enum RequestType : uint32_t
		{
			kRegisterTakenHandler = 0x100,
			kRegisterSelectHandler = 0x101,
			kRegisterLootMenuHandler = 0x102,
		};

	public:
		QuickLootAPI(QuickLootAPI const&) = delete;
		QuickLootAPI(QuickLootAPI const&&) = delete;
		QuickLootAPI operator=(QuickLootAPI&) = delete;
		QuickLootAPI operator=(QuickLootAPI&&) = delete;

		// The client initialization must happen at (or after) kPostLoad
		static void Init()
		{
			_client.Init(SERVER_PLUGIN_NAME, API_MAJOR_VERSION, API_MINOR_VERSION);
		}

		static bool IsReady()
		{
			return _client.IsReady();
		}

		static bool RegisterTakenHandler(TakenHandler::OnTakenHandler handler)
		{
			bool response = false;
			const TakenHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterTakenHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterSelectHandler(SelectHandler::OnSelectHandler handler)
		{
			bool response = false;
			const SelectHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterSelectHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterLootMenuHandler(LootMenuHandler::OnLootMenuHandler handler)
		{
			bool response = false;
			const LootMenuHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterLootMenuHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}
	};
}
