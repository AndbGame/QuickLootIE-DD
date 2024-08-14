#include "InterfaceQuickLootIE.h"

#include <QuickLootAPI.h>
#include <PluginRequests\RequestClient.h>
#include "Config.h"
#include "Manager.h"

namespace QuickLootDD
{
	static inline std::string loggerHandlerTaken(const QuickLoot::Element* elements, std::size_t elementsCount)
    {
		std::string items = "";
		for (std::size_t i = 0; i < elementsCount; ++i) {
			items += fmt::format("[{:08X}:{} {}", elements[i].object->GetFormID(), elements[i].object->GetName(), elements[i].count);
			if (elements[i].container) {
				items += fmt::format(" from <{:08X}:{}", elements[i].container->GetFormID(), elements[i].container->GetName());
				auto owner = elements[i].container->GetOwner();
				if (owner) {
					items += fmt::format("/<{:08X}:{}> ", owner->GetFormID(), owner->GetName());
				}
				items += ">]";
			}
			items += "], ";
		}
		return items;
    }

    static inline void loggerHandlerTaken(QuickLoot::TakeItemEvent* evt)
	{
		LOG("Actor <{:08X}:{}> take : {}", evt->actor->GetFormID(), evt->actor->GetName(), loggerHandlerTaken(evt->elements, evt->elementsCount));
	};

	static inline void loggerHandlerSelect(QuickLoot::SelectItemEvent* evt)
	{
		LOG("Actor <{:08X}:{}> select : {}", evt->actor->GetFormID(), evt->actor->GetName(), loggerHandlerTaken(evt->elements, evt->elementsCount));
	}

	static inline void loggerHandlerLootOpenMenu(QuickLoot::OpenLootMenuEvent* evt)
	{
		std::string source = "";
		if (evt->container) {
			source += fmt::format("[<{:08X}:{}>]", evt->container->GetFormID(), evt->container->GetName());
		} else {
			source = "NO CONTAINER";
		}
		LOG("Loot menu opened {}", source);
	}
	static inline void loggerHandlerLootCloseMenu(QuickLoot::CloseLootMenuEvent*)
	{
		LOG("Loot menu closed");
	}
	static inline void loggerHandlerInvalidateLootMenu(QuickLoot::InvalidateLootMenuEvent* evt)
	{
		LOG("Loot menu invalidated : {}", loggerHandlerTaken(evt->elements, evt->elementsCount));
	}

	static inline void TakeHandler(QuickLoot::TakeItemEvent* evt)
	{
		if (evt->actor && evt->elementsCount > 0) {
			QuickLootDD::Manager::onQLDoTake(evt->actor, evt->container, evt->elements, evt->elementsCount);
        }
	};

	static inline void SelectHandler(QuickLoot::SelectItemEvent* evt)
	{
		QuickLootDD::Manager::onQLDoSelect(evt->actor, evt->container, evt->elements, evt->elementsCount);
	};

	static inline void OpenLootMenuHandler(QuickLoot::OpenLootMenuEvent* evt)
	{
		QuickLootDD::Manager::onQLDoOpened(evt->container);
	}
	static inline void OpeningLootMenuHandler(QuickLoot::OpeningLootMenuEvent* evt)
	{
		evt->result = QuickLootDD::Manager::onQLDoOpening(evt->container);
	}
	static inline void CloseLootMenuHandler(QuickLoot::CloseLootMenuEvent*)
	{
		QuickLootDD::Manager::onQLDoClosed();
	}
	static inline void InvalidateLootMenuHandler(QuickLoot::InvalidateLootMenuEvent* evt)
	{
		QuickLootDD::Manager::onQLDoInvalidated(evt->container, evt->elements, evt->elementsCount);
	}

	bool InterfaceQuickLootIE::Init()
	{
		QuickLoot::QuickLootAPI::Init();
        if (!QuickLoot::QuickLootAPI::IsReady()) {
			ERROR("Integration with QuickLootAPI is not ready");
        }
		if (QuickLootDD::Config::QuickLootLogger) {
			if (!QuickLoot::QuickLootAPI::RegisterTakeItemHandler(loggerHandlerTaken)) {
				ERROR("loggerHandlerTaken for QuickLootAPI not registered");
			}
			if (!QuickLoot::QuickLootAPI::RegisterSelectItemHandler(loggerHandlerSelect)) {
				ERROR("loggerHandlerSelect for QuickLootAPI not registered");
			}
			if (!QuickLoot::QuickLootAPI::RegisterOpenLootMenuHandler(loggerHandlerLootOpenMenu)) {
				ERROR("loggerHandlerLootOpenMenu for QuickLootAPI not registered");
			}
			if (!QuickLoot::QuickLootAPI::RegisterInvalidateLootMenuHandler(loggerHandlerInvalidateLootMenu)) {
				ERROR("loggerHandlerInvalidateLootMenu for QuickLootAPI not registered");
			}
			if (!QuickLoot::QuickLootAPI::RegisterCloseLootMenuHandler(loggerHandlerLootCloseMenu)) {
				ERROR("loggerHandlerLootCloseMenu for QuickLootAPI not registered");
			}
		}

		if (!QuickLoot::QuickLootAPI::RegisterTakeItemHandler(TakeHandler)) {
			ERROR("TakenHandler for QuickLootAPI not registered");
		}
		if (!QuickLoot::QuickLootAPI::RegisterSelectItemHandler(SelectHandler)) {
			ERROR("SelectHandler for QuickLootAPI not registered");
		}
		if (!QuickLoot::QuickLootAPI::RegisterOpenLootMenuHandler(OpenLootMenuHandler)) {
			ERROR("OpenLootMenuHandler for QuickLootAPI not registered");
		}
		if (!QuickLoot::QuickLootAPI::RegisterOpeningLootMenuHandler(OpeningLootMenuHandler)) {
			ERROR("OpeningLootMenuHandler for QuickLootAPI not registered");
		}
		if (!QuickLoot::QuickLootAPI::RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler)) {
			ERROR("InvalidateLootMenuHandler for QuickLootAPI not registered");
		}
		if (!QuickLoot::QuickLootAPI::RegisterCloseLootMenuHandler(CloseLootMenuHandler)) {
			ERROR("CloseLootMenuHandler for QuickLootAPI not registered");
		}

		
		return true;
    }
}