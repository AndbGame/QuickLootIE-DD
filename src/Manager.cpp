#include "Manager.h"

#include "Config.h"
#include "UI.h"
#include "InterfaceDEC.h"
#include "Utils.h"

namespace QuickLootDD
{
	bool Manager::Init()
	{
		return true;
	}

	bool Manager::LoadForms()
	{
		RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

#define LOAD_FORM(NAME, TYPE, ID, PLUGIN)                                                 \
	    NAME = handler->LookupForm<TYPE>(ID, PLUGIN);                                         \
	    if (NAME == nullptr) {                                                                \
		    ERROR("LoadForms : Not found <" #TYPE " - " #ID "> '" #NAME "' in '" PLUGIN "'"); \
		    return false;                                                                     \
	    }

		itemChances.clear();

		for (auto it = QuickLootDD::Config::rawItemDefinitions.cbegin(); it != QuickLootDD::Config::rawItemDefinitions.cend(); ++it) {
			if (it->second.chance >= 0 && it->second.formId != 0 && it->second.plugin.size() > 0) {
				auto formId = handler->LookupFormID(it->second.formId, it->second.plugin);
				if (formId > 0) {
					itemChances[formId] = it->second.chance;
					DEBUG("LoadForms : Chance for {} <{:08X}> is '{}'", it->first, formId, itemChances[formId]);
				} else {
					ERROR("LoadForms : Not found Form <{:08X}> in '{}' for '{}'", it->second.formId, it->second.plugin, it->first);
				}
			} else {
				ERROR("LoadForms : Incorrect Form definition <{:08X}> in '{}' for '{}' with chance {}", it->second.formId, it->second.plugin, it->first, it->second.chance);
			}
		}

#undef LOAD_FORM
		return true;
	}

	void Manager::onQLDoTaked(RE::Actor* actor, TakenElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container)
	{

		if (!trySetBusy()) {
			DEBUG("Manager::onQLDoTaked is busy");
			return;
		}

		if (!isAllowedActor(actor)) {
			setFree();
			return;
		}

		if (!isAllowedActorLocation(actor)) {
			setFree();
			return;
		}

		if (!isContainerAllowed(container)) {
			setFree();
			return;
		}

		ContainerData contData;

        if (!containerList.getContainerData(container, &contData, true)) {
			setFree();
			return;
        }

		if (!isTriggerAllowed(container, &contData)) {
			setFree();
			return;
		}

		auto chances = getTakeLootChance(actor, container, &contData, elements, elementsCount);

		if (chances.size() <= 0) {
			setFree();
			return;
		}

		bool trigger = false;

		for (const double chance : chances) {
			if (chance > 0) {
				if (chance >= 1) {
					trigger = true;
					break;
				}
				if (Utils::randomChance(chance)) {
					trigger = true;
					break;
				}
			}
		}

		if (!trigger) {
			incContainerChance(container, &contData);
			containerList.Invalidate();
			setFree();
			return;
		}

		containerList.setTriggered(container, true);
		containerList.setContainerChance(container, 0, true);

		// Fire DEC
		LOG("onQLDoTaked Fire DEC");
		InterfaceDeviouslyEnchantedChests::TriggerTrap(container);

		containerList.Invalidate();

		setFree();
	}

	void Manager::onQLDoSelect(RE::Actor* actor, SelectElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container)
	{
		if (!QuickLootDD::Config::visualiseChance) {
			return;
		}

		if (!isAllowedActor(actor)) {
			UI::Close();
			return;
		}

		if (!isAllowedActorLocation(actor)) {
			UI::Close();
			return;
		}

		if (!isContainerAllowed(container)) {
			UI::Close();
			return;
		}

		UIInfoData infoData;

		ContainerData contData;

		if (!containerList.getContainerData(container, &contData, true)) {
			setFree();
			return;
		}

		if (!isTriggerAllowed(container, &contData)) {
			infoData.isCooldown = true;
			UI::ShowDECInfo(infoData);
			return;
		}

		auto chance = getSelectLootChance(actor, container, &contData, elements, elementsCount, &infoData);
		setFree();
		if (chance.size() > 0) {
			UI::ShowDECInfo(infoData);
		}
	}

	void Manager::onQLDoOpened(RE::Actor*, RE::TESObjectREFR*)
	{
	}

	void Manager::onQLDoClosed(RE::Actor*, RE::TESObjectREFR*)
	{
		UI::Close();
	}

	void Manager::onQLDoInvalidated(RE::Actor*, RE::TESObjectREFR* container, LootMenuElement*, std::size_t elementsCount)
	{
		if (!container) {
			UI::Close();
			DEBUG("onQLDoInvalidated container is nullptr");
			return;
		}
		if (elementsCount <= 0) {
			UI::Close();
		}

		if (elementsCount <= 0 && QuickLootDD::Config::resetChanceOnLastItem) {
			containerList.setContainerChance(container, 0, true);
		}
	}

    std::vector<double> Manager::getTakeLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, TakenElement* element, std::size_t elementsCount)
	{
		std::vector<double> chances;

		auto containerChance = getContainerChance(container, contData);
		auto locationChance = getLocationChance(actor);

		for (std::size_t i = 0; i < elementsCount; ++i) {
			chances.push_back(getItemChance(containerChance * locationChance, element[i].object, element[i].count));
		}
		return chances;
	}

	std::vector<double> Manager::getSelectLootChance(RE::Actor* actor, RE::TESObjectREFR* container, ContainerData* contData, SelectElement* element, std::size_t elementsCount, UIInfoData* infoData)
	{
		std::vector<double> chances;

		auto containerChance = getContainerChance(container, contData);
		auto locationChance = getLocationChance(actor);

		infoData->containerChance = containerChance;
		infoData->locationChance = locationChance;

		for (std::size_t i = 0; i < elementsCount; ++i) {
			UIItemInfoData itemInfoData;
			chances.push_back(getItemChance(containerChance * locationChance, element[i].object, element[i].count, &itemInfoData));
			if (i < UIItemInfoDataLength) {
				infoData->itemChance[i] = itemInfoData;
				infoData->itemCount = i + 1;
			}
		}
		return chances;
	}

	double Manager::getItemChance(const double baseChance, RE::TESForm* object, std::size_t elementsCount, UIItemInfoData* itemInfoData)
	{
		double itemChance = 1.0;

		if (!QuickLootDD::Config::useItemChanceMultiplier) {
			if (itemInfoData != nullptr) {
				itemInfoData->count = 1;
				itemInfoData->itemChance = 1.0;
				itemInfoData->totalChance = baseChance;
			}
			return baseChance;
		}

		if (auto search = itemChances.find(object->GetFormID()); search != itemChances.end()) {
			itemChance = search->second;
		} else {
			switch (object->formType.get()) {
			case RE::FormType::Scroll:
				itemChance = QuickLootDD::Config::ScrollItemChanceMultiplier;
				break;
			case RE::FormType::Armor:
				itemChance = QuickLootDD::Config::ArmorItemChanceMultiplier;
				break;
			case RE::FormType::Book:
				itemChance = QuickLootDD::Config::BookItemChanceMultiplier;
				break;
			case RE::FormType::Ingredient:
				itemChance = QuickLootDD::Config::IngredientItemChanceMultiplier;
				break;
			case RE::FormType::Light:
				itemChance = QuickLootDD::Config::LightItemChanceMultiplier;
				break;
			case RE::FormType::Misc:
				itemChance = QuickLootDD::Config::MiscItemChanceMultiplier;
				break;
			case RE::FormType::Weapon:
				itemChance = QuickLootDD::Config::WeaponItemChanceMultiplier;
				break;
			case RE::FormType::Ammo:
				itemChance = QuickLootDD::Config::AmmoItemChanceMultiplier;
				break;
			case RE::FormType::KeyMaster:
				itemChance = QuickLootDD::Config::KeyMasterItemChanceMultiplier;
				break;
			case RE::FormType::AlchemyItem:
				itemChance = QuickLootDD::Config::AlchemyItemChanceMultiplier;
				break;
			case RE::FormType::SoulGem:
				itemChance = QuickLootDD::Config::SoulGemItemChanceMultiplier;
				break;
			}
		}

		if (itemInfoData != nullptr) {
			if (QuickLootDD::Config::useCountOfItemsChanceMultiplier) {
				itemInfoData->count = elementsCount;
			} else {
				itemInfoData->count = 1;
			}
			itemInfoData->itemChance = itemChance;
		}

		itemChance *= baseChance;
		if (itemChance <= 0) {
			if (itemInfoData != nullptr) {
				itemInfoData->totalChance = 0.0;
			}
			return 0.0;
		}
		if (itemChance >= 1) {
			if (itemInfoData != nullptr) {
				itemInfoData->totalChance = 1.0;
			}
			return 1.0;
		}

		if (QuickLootDD::Config::useCountOfItemsChanceMultiplier) {
			if (elementsCount <= 0) {
				return 0.0;
			}
			itemChance = 1 - std::pow((1 - itemChance), elementsCount);
		}
		if (itemInfoData != nullptr) {
			itemInfoData->totalChance = itemChance;
		}

		return itemChance;
	}

	double Manager::getLocationChance(RE::Actor* actor)
	{
		auto location = actor->GetCurrentLocation();
		auto parentCell = actor->GetParentCell();

		if (location && location->HasAnyKeywordByEditorID(QuickLootDD::Config::CityLocations)) {
			//DEBUG("getLocationChance CityLocation");
			return QuickLootDD::Config::CityChanceMultiplier;
		}

		if (!parentCell || !parentCell->IsInteriorCell()) {
			if (!location || location->HasAnyKeywordByEditorID(QuickLootDD::Config::WildernessLocations)) {
				//DEBUG("getLocationChance WildernessLocation");
				return QuickLootDD::Config::WildernessChanceMultiplier;
			}
		}

		if (parentCell && parentCell->IsInteriorCell() && location && location->HasAnyKeywordByEditorID(QuickLootDD::Config::DungeonLocations)) {
			//DEBUG("getLocationChance DungeonLocation");
			return QuickLootDD::Config::DungeonChanceMultiplier;
		}
		//DEBUG("getLocationChance Unknown Location");

		return 1.0;
	}

	bool Manager::isTriggerAllowed(RE::TESObjectREFR* /*container*/, ContainerData* contData)
	{
		auto now = RE::Calendar::GetSingleton()->GetDaysPassed();

		if ((now * 24 * 60 * 60) < ((contData->lastTriggered * 24 * 60 * 60) + (QuickLootDD::Config::containerTriggerCooldown))) {
			//DEBUG("isTriggerAllowed container cooldown");
			return false;
		}

		return true;
	}

	inline bool Manager::isAllowedActor(RE::Actor* actor)
	{
		return actor == RE::PlayerCharacter::GetSingleton() && actor->HasKeywordString(KeywordId.ActorTypeNPC);
	}

	inline bool Manager::isAllowedActorLocation(RE::Actor* actor)
	{
		auto location = actor->GetCurrentLocation();
		if (!location) {
			return true;
		}
		if (location->HasAnyKeywordByEditorID(QuickLootDD::Config::SafeLocations)) {
			//DEBUG("getLocationChance SafeLocation");
			return false;
		}
		return true;
	}

	inline bool Manager::isContainerAllowed(RE::TESObjectREFR* container)
	{
		if (container == nullptr) {
			DEBUG("isContainerAllowed Container in nullptr");
			return false;
		}

		if (container->IsLocked()) {
			DEBUG("isContainerAllowed Container is Locked");
			return false;
		}

		if (auto actor = container->As<RE::Actor>(); actor) {
			if ((actor->IsDead() && !actor->IsSummoned()) && QuickLootDD::Config::allowDeadBody) {
				if (actor->HasKeywordString("ActorTypeNPC")) {
					return QuickLootDD::Config::allowActorTypeNPC;
				}
				if (actor->HasKeywordString("ActorTypeAnimal")) {
					return QuickLootDD::Config::allowActorTypeAnimal;
				}
				if (actor->HasKeywordString("ActorTypeDragon")) {
					return QuickLootDD::Config::allowActorTypeDragon;
				}
				if (actor->HasKeywordString("ActorTypeCreature")) {
					return QuickLootDD::Config::allowActorTypeCreature;
				}
				return QuickLootDD::Config::allowOther;
			}
		}

		auto baseObjContainer = container->GetBaseObject();
		if (baseObjContainer == nullptr) {
			//DEBUG("isContainerAllowed Base container in nullptr");
			return false;
		}

        if (!InterfaceDeviouslyEnchantedChests::isContainerAllowed(baseObjContainer)) {
			DEBUG("isContainerAllowed <{:08X}:{}> not allowed by DEC", baseObjContainer->GetFormID(), baseObjContainer->GetName());
			return false;
		}
		return true;
	}

	double Manager::getContainerChance(RE::TESObjectREFR* container, ContainerData* contData)
	{
		auto now = RE::Calendar::GetSingleton()->GetDaysPassed();

        if (contData->chance == 0 || (now * 24 * 60 * 60) > ((contData->lastUsed * 24 * 60 * 60) + (QuickLootDD::Config::containerChanceCooldown))) {
			generateContainerChance(container, contData);
        }

		return contData->chance;
	}

	void Manager::incContainerChance(RE::TESObjectREFR* container, ContainerData* contData)
	{
		double containerChance = getContainerChance(container, contData) * Utils::randomValue(QuickLootDD::Config::increaseChanceMultiplierMin, QuickLootDD::Config::increaseChanceMultiplierMax);
		if (containerChance > QuickLootDD::Config::chanceLimit) {
			containerChance = QuickLootDD::Config::chanceLimit;
		}
		contData->chance = containerChance;
		containerList.setContainerChance(container, contData->chance);
	}

	void Manager::decContainerChance(RE::TESObjectREFR* container, ContainerData* contData)
	{
		double containerChance = getContainerChance(container, contData) / Utils::randomValue(QuickLootDD::Config::increaseChanceMultiplierMin, QuickLootDD::Config::increaseChanceMultiplierMax);
		if (containerChance < 0) {
			containerChance = 0;
		}
		contData->chance = containerChance;
		containerList.setContainerChance(container, contData->chance);
	}

	void Manager::generateContainerChance(RE::TESObjectREFR* container, ContainerData* contData)
	{
		contData->chance = Utils::randomValue(QuickLootDD::Config::baseChanceMin, QuickLootDD::Config::baseChanceMax);
		if (contData->chance > QuickLootDD::Config::chanceLimit) {
			contData->chance = QuickLootDD::Config::chanceLimit;
		}
		containerList.setContainerChance(container, contData->chance);
	}

	void Manager::RevertState(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Manager::RevertState");
		containerList.RevertState(serializationInterface);
		InterfaceDeviouslyEnchantedChests::RevertState(serializationInterface);
		isBusy.store(false);
	}

	void Manager::SaveState(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Manager::SaveState");
		/* 1) */ containerList.SaveState(serializationInterface);
	}

	void Manager::LoadState(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Manager::LoadState");
		/* 1) */ containerList.LoadState(serializationInterface);
	}

	bool Manager::trySetBusy()
	{
		auto expected = false;
		return isBusy.compare_exchange_strong(expected, true);
	}

	void Manager::setFree()
	{
		isBusy.store(false);
	}
}