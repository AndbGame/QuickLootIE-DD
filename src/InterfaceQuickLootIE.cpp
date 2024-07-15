#include "InterfaceQuickLootIE.h"

#include <QuickLootIEInterface.h>

namespace QuickLootDD
{
	InterfaceDeviouslyEnchantedChests* InterfaceQuickLootIE::_dec = nullptr;

    void loggerHandler(QuickLootIE::QuickLootIEInterface::TakedEvent evt) {
		std::string take = "take";
		if (evt.isStealAlarm) {
			take = "steal";
		}
		std::string source = "";
        if (evt.source == QuickLootIE::QuickLootIEInterface::TakedSource::GROUNG) {
			source = "GROUND";
		}
		if (evt.source == QuickLootIE::QuickLootIEInterface::TakedSource::CONTAINER) {
			source = "CONTAINER";
			if (evt.container) {
				source += fmt::format(" <{:08X}:{}>", evt.container->GetFormID(), evt.container->GetName());
			}
			if (evt.containerOwner) {
				source += fmt::format("/<{:08X}:{}> ", evt.containerOwner->GetFormID(), evt.containerOwner->GetName());
			}
		}
		std::string items = "";
        for (auto const& item : evt.items) {
			items += fmt::format("[{:08X}:{}:{}], ", item.object->GetFormID(), item.object->GetName(), item.count);
        }
		LOG("Actor <{:08X}:{}> {} : {} from [{}]", evt.actor->GetFormID(), evt.actor->GetName(), take, items, source);
	};

	void DECHandler(QuickLootIE::QuickLootIEInterface::TakedEvent evt)
	{
		auto dec = InterfaceQuickLootIE::getDEC();
		if (dec && evt.actor && evt.container && evt.items.size() > 0) {
			dec->onQLDoTaked(evt.actor, evt.items, evt.container, evt.containerOwner, evt.isStealAlarm);
        }
	};

	bool InterfaceQuickLootIE::Init(const SKSE::MessagingInterface* messagingInterface)
	{
		QuickLootIE::InterfaceMessage data;
		messagingInterface->Dispatch(QuickLootIE::MessageType::Interface, &data, sizeof(data), "QuickLootIE");

        if (!data.quickLootInterface) {
			ERROR("empty QuickLootInterface: incorrect version or missed QuickLootIE plugin");
			return false;
        }

        if (data.quickLootInterface->getMajorVersion() != 2) {
			ERROR("Incorrect version of QuickLootInterface");
			return false;
        }
		data.quickLootInterface->registerOnTakedHandler(&loggerHandler);
		data.quickLootInterface->registerOnTakedHandler(&DECHandler);
		
		return true;
    }
}