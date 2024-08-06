#pragma once

#include <QuickLootIntegrations.h>

#include "ContainerList.h"

namespace QuickLootDD
{
    using Element = QuickLoot::Integrations::Element;
    struct UIInfoData;
	struct UIItemInfoData;

	struct BonusItem
	{
		RE::FormID formId;
		size_t count = 1;
		double simpleChance = 0;
		double chanceToReplaceRestraint = 0;
	};

	class Manager
	{
	public:
		static bool Init();
		static bool LoadForms();

		static void onQLDoTake(RE::Actor* actor, RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoSelect(RE::Actor* actor, RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoOpened(RE::TESObjectREFR* container);
		static QuickLoot::Integrations::OpeningLootMenuHandler::HandleResult onQLDoOpening(RE::TESObjectREFR* container);
		static void onQLDoInvalidated(RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoClosed();
        
		static void RevertState(SKSE::SerializationInterface* serializationInterface);
		static void SaveState(SKSE::SerializationInterface* serializationInterface);
		static void LoadState(SKSE::SerializationInterface* serializationInterface);

	protected:
		static inline std::atomic<bool> isBusy = false;
		static inline std::map<RE::FormID, double> itemChances;
		static inline std::vector<BonusItem> bonusLoot;
		static inline ContainerList containerList;
		struct
		{
			std::string_view ActorTypeNPC = "ActorTypeNPC";
		} static inline KeywordId;

        static bool trySetBusy();
		static void setFree();

		static bool isAllowedActor(RE::Actor* actor);
		static bool isAllowedActorLocation(RE::Actor* actor);
		static bool isContainerAllowed(RE::TESObjectREFR* container);
		static bool isTriggerAllowed(RE::TESObjectREFR* container, ContainerData* contData);

		static std::vector<double> getTakeLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, Element* element, std::size_t elementsCount);
		static std::vector<double> getSelectLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, Element* element, std::size_t elementsCount, UIInfoData* infoData);
		static double getItemChance(const double baseChance, RE::TESForm* object, std::size_t elementsCount, UIItemInfoData* itemInfoData = nullptr);
		static double getLocationChance(RE::Actor* actor);

		static double getContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void incContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void decContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void generateContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
	};
}