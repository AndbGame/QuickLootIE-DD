#include "InterfaceQuickLootIE.h"

#include <QuickLootIntegrations.h>
#include <PluginRequests\RequestClient.h>
#include "Config.h"

namespace QuickLootDD
{

    static void loggerHandlerTaken(QuickLoot::Integrations::TakenHandler::TakenEvent* evt) {
		std::string take = "take";
		/*if (evt.isStealAlarm) {
			take = "steal";
		}*/
		std::string source = "";
		if (evt->container) {
			source += fmt::format(" from [<{:08X}:{}>", evt->container->GetFormID(), evt->container->GetName());
			auto owner = evt->container->GetOwner();
			if (owner) {
				source += fmt::format("/<{:08X}:{}> ", owner->GetFormID(), owner->GetName());
			}
			source += "]";
		}
		std::string items = "";
		for (std::size_t i = 0; i < evt->elementsCount; ++i) {
			items += fmt::format("[{:08X}:{} {}], ", evt->elements[i].object->GetFormID(), evt->elements[i].object->GetName(), evt->elements[i].count);
        }
		LOG("Actor <{:08X}:{}> {} : {}{}", evt->actor->GetFormID(), evt->actor->GetName(), take, items, source);
	};

	static void loggerHandlerSelect(QuickLoot::Integrations::SelectHandler::SelectEvent* evt)
	{
		std::string source = "";
		if (evt->container) {
			source += fmt::format("[<{:08X}:{}>]", evt->container->GetFormID(), evt->container->GetName());
		} else {
			source = "NO CONTAINER";
		}
		std::string items = "";
		for (std::size_t i = 0; i < evt->elementsCount; ++i) {
			items += fmt::format("[{:08X}:{} - {}], ", evt->elements[i].object->GetFormID(), evt->elements[i].object->GetName(), evt->elements[i].count);
		}
		LOG("Actor <{:08X}:{}> Loot menu selected: container - {}; elements - {}", evt->actor->GetFormID(), evt->actor->GetName(), source, items);
	}

	static void loggerHandlerLootMenu(QuickLoot::Integrations::LootMenuHandler::LootMenuEvent* evt)
	{
        if (evt->status == QuickLoot::Integrations::LootMenuHandler::Status::CLOSE) {
			LOG("Actor <{:08X}:{}> : Loot menu closed", evt->actor->GetFormID(), evt->actor->GetName());
		}
		if (evt->status == QuickLoot::Integrations::LootMenuHandler::Status::OPEN) {
			std::string source = "";
			if (evt->container) {
				source += fmt::format("[<{:08X}:{}>]", evt->container->GetFormID(), evt->container->GetName());
			} else {
				source = "NO CONTAINER";
			}
			LOG("Actor <{:08X}:{}> : Loot menu opened {}", evt->actor->GetFormID(), evt->actor->GetName(), source);
		}
		if (evt->status == QuickLoot::Integrations::LootMenuHandler::Status::INVALIDATE) {
			std::string source = "";
			if (evt->container) {
				source += fmt::format("[<{:08X}:{}>]", evt->container->GetFormID(), evt->container->GetName());
			} else {
				source = "NO CONTAINER";
			}
			std::string items = "";
			for (std::size_t i = 0; i < evt->elementsCount; ++i) {
				items += fmt::format("[{:08X}:{} {}], ", evt->elements[i].object->GetFormID(), evt->elements[i].object->GetName(), evt->elements[i].count);
			}
			LOG("Actor <{:08X}:{}> Loot menu invalidated: container - {}; elements - {}", evt->actor->GetFormID(), evt->actor->GetName(), source, items);
        }
	}

	void DECTakenHandler(QuickLoot::Integrations::TakenHandler::TakenEvent* evt)
	{
		if (evt->actor && evt->container && evt->elementsCount > 0) {
			QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoTaked(evt->actor, evt->elements, evt->elementsCount, evt->container);
        }
	};

	void DECLootMenuHandler(QuickLoot::Integrations::LootMenuHandler::LootMenuEvent* evt)
	{
		switch (evt->status) {
		case QuickLoot::Integrations::LootMenuHandler::Status::INVALIDATE:
			QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoInvalidated(evt->actor, evt->container, evt->elements, evt->elementsCount);
			break;
		case QuickLoot::Integrations::LootMenuHandler::Status::OPEN:
			QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoOpened(evt->actor, evt->container);
			break;
		case QuickLoot::Integrations::LootMenuHandler::Status::CLOSE:
			QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoClosed(evt->actor, evt->container);
			break;
        }
	};

	void DECSelectHandler(QuickLoot::Integrations::SelectHandler::SelectEvent* evt)
	{
		QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoSelect(evt->actor, evt->elements, evt->elementsCount, evt->container);
	};

	bool InterfaceQuickLootIE::Init()
	{
		QuickLoot::Integrations::QuickLootAPI::Init();
        if (!QuickLoot::Integrations::QuickLootAPI::IsReady()) {
			ERROR("Integration with QuickLootAPI is not ready");
        }
		if (QuickLootDD::Config::QuickLootLogger) {
			if (!QuickLoot::Integrations::QuickLootAPI::RegisterTakenHandler(loggerHandlerTaken)) {
				ERROR("loggerHandlerTaken for QuickLootAPI not registered");
			}
			if (!QuickLoot::Integrations::QuickLootAPI::RegisterSelectHandler(loggerHandlerSelect)) {
				ERROR("loggerHandlerSelect for QuickLootAPI not registered");
			}
			if (!QuickLoot::Integrations::QuickLootAPI::RegisterLootMenuHandler(loggerHandlerLootMenu)) {
				ERROR("loggerHandlerLootMenu for QuickLootAPI not registered");
			}
		}

		if (!QuickLoot::Integrations::QuickLootAPI::RegisterTakenHandler(DECTakenHandler)) {
			ERROR("DECTakenHandler for QuickLootAPI not registered");
		}

		if (!QuickLoot::Integrations::QuickLootAPI::RegisterLootMenuHandler(DECLootMenuHandler)) {
			ERROR("DECLootMenuHandler for QuickLootAPI not registered");
		}

		if (!QuickLoot::Integrations::QuickLootAPI::RegisterSelectHandler(DECSelectHandler)) {
			ERROR("DECSelectHandler for QuickLootAPI not registered");
		}
		
		return true;
    }
}