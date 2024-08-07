#pragma once

#include <QuickLootIntegrations.h>

#include "ContainerList.h"
#include "Config.h"

namespace QuickLootDD
{
    using Element = QuickLoot::Integrations::Element;
    using BonusItemFlag = Config::BonusItemDefinition::RequirementFlags;

    struct UIInfoData;
	struct UIItemInfoData;

	struct BonusItem
	{
		RE::TESBoundObject* object;
		std::int32_t minCount = 1;
		std::int32_t maxCount = 1;
		double chance = 0;
		stl::enumeration<BonusItemFlag, std::uint32_t> requirement = BonusItemFlag::NONE;
	};

    struct PlayerEquipmentInfo
	{
		bool lockable = false;
		bool belt = false;
		bool bra = false;
		bool piercing = false;
		bool plug = false;
		bool heavyBondage = false;
		bool bondageMittens = false;
	};

	struct BonusItemQuery
	{
		PlayerEquipmentInfo DDEquipment;
	};

	class Manager
	{
	public:
		static bool Init();
		static void installEventSink();
		static bool LoadForms();

		static void onQLDoTake(RE::Actor* actor, RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoSelect(RE::Actor* actor, RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoOpened(RE::TESObjectREFR* container);
		static QuickLoot::Integrations::OpeningLootMenuHandler::HandleResult onQLDoOpening(RE::TESObjectREFR* container);
		static void onQLDoInvalidated(RE::TESObjectREFR* container, Element* elements, std::size_t elementsCount);
		static void onQLDoClosed();
		static void invalidateEquipment();
        
		static void RevertState(SKSE::SerializationInterface* serializationInterface);
		static void SaveState(SKSE::SerializationInterface* serializationInterface);
		static void LoadState(SKSE::SerializationInterface* serializationInterface);

	protected:
		static inline std::atomic<bool> isBusy = false;
		static inline std::atomic<bool> invalidateEquip = true;
		static inline std::map<RE::FormID, double> itemChances;
		static inline std::vector<BonusItem> bonusLoot;
		static inline ContainerList containerList;
		static inline PlayerEquipmentInfo playerEquipmentInfo;
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

        static std::vector<std::tuple<RE::TESBoundObject*, std::int32_t>> getBonusItem(BonusItemQuery* query);
		static PlayerEquipmentInfo getPlayerEquipmentInfo();
	};
}