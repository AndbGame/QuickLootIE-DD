#pragma once

#include <QuickLootIEInterface.h>

namespace QuickLootDD
{
    using TakedItem = QuickLootIE::QuickLootIEInterface::TakedItem;
    using clock = std::chrono::high_resolution_clock;

	class InterfaceDeviouslyEnchantedChests
	{
	public:
		InterfaceDeviouslyEnchantedChests();

		bool Init();
		bool LoadForms();
		bool Reset();

        void onQLDoTaked(RE::Actor* actor,
			std::vector<TakedItem> items,
			RE::TESObjectREFR* container,
			RE::TESForm* containerOwner,
			bool isStealAlarm);

	protected:
		RE::TESQuest* dtraps_Quest = nullptr;
		RE::BGSListForm* dt_containerformlist = nullptr;
		RE::TESFaction* CurrentFollowerFaction = nullptr;
		RE::TESFaction* CurrentHireling = nullptr;
		struct
		{
			std::string_view ActorTypeNPC = "ActorTypeNPC";
		} KeywordId;

		std::atomic<bool> isReady = false;
		std::atomic<bool> isBusy = false;
		std::map<RE::FormID, clock::time_point> _lastLootedContainers;
		clock::time_point _lastTriggered;

        std::vector<RE::Actor*> getNearestFollowers(RE::Actor* actor);
		bool isAllowedContainer(RE::TESObjectREFR* container);
		bool isAllowedActor(RE::Actor* actor);
	};
}