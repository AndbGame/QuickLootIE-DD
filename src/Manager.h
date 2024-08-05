#pragma once

#include <QuickLootIntegrations.h>

#include "ContainerList.h"

namespace QuickLootDD
{
    using TakenElement = QuickLoot::Integrations::TakenHandler::Element;
    using SelectElement = QuickLoot::Integrations::SelectHandler::Element;
    using LootMenuElement = QuickLoot::Integrations::LootMenuHandler::Element;

    struct UIInfoData;
	struct UIItemInfoData;

	class Manager
	{
	public:
		static bool Init();
		static bool LoadForms();

		static void onQLDoTaked(RE::Actor* actor, TakenElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container);
		static void onQLDoSelect(RE::Actor* actor, SelectElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container);
		static void onQLDoOpened(RE::Actor* actor, RE::TESObjectREFR* container);
		static void onQLDoInvalidated(RE::Actor* actor, RE::TESObjectREFR* container, LootMenuElement* elements, std::size_t elementsCount);
		static void onQLDoClosed(RE::Actor* actor, RE::TESObjectREFR* container);
        
		static void RevertState(SKSE::SerializationInterface* serializationInterface);
		static void SaveState(SKSE::SerializationInterface* serializationInterface);
		static void LoadState(SKSE::SerializationInterface* serializationInterface);

	protected:
		static inline std::atomic<bool> isBusy = false;
		static inline std::map<RE::FormID, double> itemChances;
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

		static std::vector<double> getTakeLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, TakenElement* element, std::size_t elementsCount);
		static std::vector<double> getSelectLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, SelectElement* element, std::size_t elementsCount, UIInfoData* infoData);
		static double getItemChance(const double baseChance, RE::TESForm* object, std::size_t elementsCount, UIItemInfoData* itemInfoData = nullptr);
		static double getLocationChance(RE::Actor* actor);

		static double getContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void incContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void decContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
		static void generateContainerChance(RE::TESObjectREFR* container, ContainerData* contData);
	};
}