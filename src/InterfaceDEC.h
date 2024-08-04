#pragma once

#include <QuickLootIntegrations.h>

namespace QuickLootDD
{
    using TakenElement = QuickLoot::Integrations::TakenHandler::Element;
    using SelectElement = QuickLoot::Integrations::SelectHandler::Element;
    using LootMenuElement = QuickLoot::Integrations::LootMenuHandler::Element;
    using clock = std::chrono::high_resolution_clock;

	class InterfaceDeviouslyEnchantedChests
	{
	public:
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
			UIItemInfoData itemChance[UIItemInfoDataLength];
			std::size_t itemCount = 0;
        };
		struct ContainerData
		{
			double chance = 0.0;
			float lastTriggered = 0.0f;
			float lastUsed = 0.0f;
        };

		static bool Init();
		static bool LoadForms();
		static bool Reset();

        static void onQLDoTaked(
            RE::Actor* actor,
			TakenElement* elements,
			std::size_t elementsCount,
			RE::TESObjectREFR* container);

		static void onQLDoSelect(
			RE::Actor* actor,
			SelectElement* elements,
			std::size_t elementsCount,
			RE::TESObjectREFR* container);

		static void onQLDoOpened(
			RE::Actor* actor,
			RE::TESObjectREFR* container);

		static void onQLDoInvalidated(
			RE::Actor* actor,
			RE::TESObjectREFR* container,
			LootMenuElement* elements,
			std::size_t elementsCount);

		static void onQLDoClosed(
			RE::Actor* actor,
			RE::TESObjectREFR* container);

		static bool isAllowedActor(RE::Actor* actor);
		static bool isAllowedActorLocation(RE::Actor* actor);
		static bool isContainerAllowed(RE::TESObjectREFR* container);
        static bool isTriggerAllowed(RE::TESObjectREFR* container);

	protected:
		//static inline RE::TESQuest* dtraps_Quest = nullptr;
		static inline RE::BGSListForm* dt_containerformlist = nullptr;
		//static inline RE::TESFaction* CurrentFollowerFaction = nullptr;
		//static inline RE::TESFaction* CurrentHireling = nullptr;
		struct
		{
			std::string_view ActorTypeNPC = "ActorTypeNPC";
		} static inline KeywordId;

		static inline std::atomic<bool> isReady = false;
		static inline std::atomic<bool> isBusy = false;
		static inline float _lastInvalidationTime = 0;
		static inline std::map<RE::FormID, ContainerData> _containerData = {};
		static inline std::map<RE::FormID, double> itemChances;

		static std::vector<double> getTakeLootChance(RE::Actor* actor, RE::TESObjectREFR* container, TakenElement* element, std::size_t elementsCount);
		static std::vector<double> getSelectLootChance(RE::Actor* actor, RE::TESObjectREFR* container, SelectElement* element, std::size_t elementsCount, UIInfoData* infoData);
		static double getItemChance(const double baseChance, RE::TESForm* object, std::size_t elementsCount, UIItemInfoData* itemInfoData = nullptr);

        static double getContainerChance(RE::TESObjectREFR* container);
		static void incContainerChance(RE::TESObjectREFR* container);
		static void decContainerChance(RE::TESObjectREFR* container);
		static double resetContainerChance(RE::TESObjectREFR* container);
		static double generateContainerChance(RE::TESObjectREFR* container);
		static void invalidateContainerData();

        static double getLocationChance(RE::Actor* actor);

        //static std::vector<RE::Actor*> getNearestFollowers(RE::Actor* actor);
	};
}