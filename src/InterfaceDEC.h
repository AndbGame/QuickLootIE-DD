#pragma once


namespace QuickLootDD
{
	class InterfaceDeviouslyEnchantedChests
	{
	public:

		static bool Init();
		static bool LoadForms();
		static void RevertState(SKSE::SerializationInterface* serializationInterface);

		static bool isContainerAllowed(const RE::TESObjectREFR* container);
		static void TriggerTrap(RE::TESObjectREFR* container);

	protected:
		static inline RE::BGSListForm* dt_containerformlist = nullptr;
		static inline RE::TESQuest* dtraps_Quest = nullptr;
		//static inline RE::TESFaction* CurrentFollowerFaction = nullptr;
		//static inline RE::TESFaction* CurrentHireling = nullptr;
		//static inline RE::TESFaction* CurrentHireling = nullptr;
	};
}