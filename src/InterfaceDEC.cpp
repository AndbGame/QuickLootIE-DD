#include "InterfaceDEC.h"

#include "Config.h"
#include "UI.h"
#include "Utils.h"
#include "Serialization.h"

namespace QuickLootDD
{
	bool InterfaceDeviouslyEnchantedChests::Init()
	{
		return true;
	}
	bool InterfaceDeviouslyEnchantedChests::LoadForms()
	{
		RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

#define LOAD_FORM(NAME, TYPE, ID, PLUGIN)                                                                      \
	    NAME = handler->LookupForm<TYPE>(ID, PLUGIN);                                                          \
	    if (NAME == nullptr) {                                                                                 \
		    ERROR("LoadForms : Not found <" #TYPE " - " #ID "> '" #NAME "' in '" PLUGIN "'");                  \
		    return false;                                                                                      \
	    }

		if (QuickLootDD::Config::useDECContainerList) {
			LOAD_FORM(dt_containerformlist, RE::BGSListForm, 0x001829, "dD Enchanted Chests.esp");
		}
		LOAD_FORM(dtraps_Quest, RE::TESQuest, 0x000D62, "dD Enchanted Chests.esp");
		//LOAD_FORM(CurrentFollowerFaction, RE::TESFaction, 0x05C84E, "Skyrim.esm");
		//LOAD_FORM(CurrentHireling, RE::TESFaction, 0x0BD738, "Skyrim.esm");

#undef LOAD_FORM
		return true;
	}

	void InterfaceDeviouslyEnchantedChests::RevertState(SKSE::SerializationInterface* /*serializationInterface*/)
	{
	}

	bool InterfaceDeviouslyEnchantedChests::isContainerAllowed(const RE::TESObjectREFR* container)
	{
		if (dtraps_Quest) {
			auto dtraps_mcm = Utils::GetScriptObject(dtraps_Quest, "dtraps_mcm");
			auto lootedlistsize = dtraps_mcm->GetProperty("lootedlistsize")->GetUInt();
			//DEBUG("InterfaceDeviouslyEnchantedChests::isContainerAllowed lootedlistsize {}/{}", dtraps_mcm->GetProperty("lootedlistsize")->GetSInt(), dtraps_mcm->GetProperty("lootedlistsize")->GetUInt());
			auto lastlooted = dtraps_mcm->GetProperty("lastlooted")->GetArray();
			int limit = lootedlistsize < lastlooted->size() ? lootedlistsize : lastlooted->size();
			for (int i = 0; i < limit; i++) {
				auto& val = lastlooted->data()[i];
				//DEBUG("InterfaceDeviouslyEnchantedChests::isContainerAllowed data {:08X}/{:08X}", val.GetSInt(), val.GetUInt());
				if (val.IsInt() && val.GetUInt() == container->GetFormID()) {
					DEBUG("InterfaceDeviouslyEnchantedChests::isContainerAllowed is lastlooted");
					return false;
				}
            }
		}
		auto baseObjContainer = container->GetBaseObject();
		if (baseObjContainer == nullptr) {
			return false;
		}
		if (QuickLootDD::Config::useDECContainerList && dt_containerformlist) {
			return dt_containerformlist->HasForm(baseObjContainer);
		}
		return true;
	}
	void InterfaceDeviouslyEnchantedChests::TriggerTrap(RE::TESObjectREFR* container)
	{
		SKSE::ModCallbackEvent dTrapEvent { "dtraps_TriggerTrap", "", 0, container };
		SKSE::GetModCallbackEventSource()->SendEvent(&dTrapEvent);
	}
}