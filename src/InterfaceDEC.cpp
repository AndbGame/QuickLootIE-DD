#include "InterfaceDEC.h"

#include "TESUtils.h"
#include "Config.h"
#include "UI.h"

namespace QuickLootDD
{
	/*
	struct EmptyRequestCallback : public RE::BSScript::IStackCallbackFunctor
	{
	public:
		EmptyRequestCallback(std::string info) { _info = info; }
		~EmptyRequestCallback() = default;

		virtual void operator()([[maybe_unused]] RE::BSScript::Variable a_result) override
		{
			DEBUG("EmptyRequestCallback - {}", _info);
		};

		virtual void SetObject([[maybe_unused]] const RE::BSTSmartPointer<RE::BSScript::Object>& a_object){};

	private:
		std::string _info;
	};
    */

	static bool setBusy(std::atomic<bool>& isBusy)
	{
		auto expected = false;
		return isBusy.compare_exchange_strong(expected, true);
	}

	static void setFree(std::atomic<bool>& isBusy)
	{
		isBusy.store(false);
	}

	static inline bool randomChance(const double chance, const double min = 0, const double max = 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr(min, max);
		auto rnd = distr(gen);
		DEBUG("randomChance chance:{}; value:{}", chance, rnd);
		return rnd < chance;
	}

	static inline double randomValue(const double min = 0, const double max = 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr(min, max);

		return distr(gen);
	}

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

		//LOAD_FORM(dtraps_Quest, RE::TESQuest, 0x000D62, "dD Enchanted Chests.esp");
		if (QuickLootDD::Config::useDECContainerList) {
			LOAD_FORM(dt_containerformlist, RE::BGSListForm, 0x001829, "dD Enchanted Chests.esp");
		}
		//LOAD_FORM(CurrentFollowerFaction, RE::TESFaction, 0x05C84E, "Skyrim.esm");
		//LOAD_FORM(CurrentHireling, RE::TESFaction, 0x0BD738, "Skyrim.esm");
		itemChances.clear();

		for (auto it = QuickLootDD::Config::rawItemDefinitions.cbegin(); it != QuickLootDD::Config::rawItemDefinitions.cend(); ++it)
		{
			if (it->second.chance >= 0 && it->second.formId != 0 && it->second.plugin.size() > 0) {
				auto formId = handler->LookupFormID(it->second.formId, it->second.plugin);
				if (formId > 0) {
					itemChances[formId] = it->second.chance;
					DEBUG("LoadForms : Chance for {} <{:08X}> in '<{:08X}:{}>' is '{}'", it->first, formId, it->second.formId, it->second.plugin, it->second.chance);
                } else {
					ERROR("LoadForms : Not found Form <{:08X}> in '{}' for '{}'", it->second.formId, it->second.plugin, it->first);
				}
			} else {
				ERROR("LoadForms : Incorrect Form definition <{:08X}> in '{}' for '{}' with chance {}", it->second.formId, it->second.plugin, it->first, it->second.chance);
			}
		}

#undef LOAD_FORM
		isReady.store(true);
		return true;
	}

	bool InterfaceDeviouslyEnchantedChests::Reset()
	{
		_containerData.clear();
		_lastInvalidationTime = RE::Calendar::GetSingleton()->GetDaysPassed();
		isBusy.store(false);
		return true;
	}

	void QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoTaked(RE::Actor* actor, TakenElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container)
	{
		if (!isReady.load(std::memory_order::relaxed)) {
			DEBUG("onQLDoTaked not loaded");
			return;
		}

		if (!setBusy(isBusy)) {
			DEBUG("onQLDoTaked is busy");
			return;
		}

		if (!isAllowedActor(actor)) {
			setFree(isBusy);
			return;
		}

		if (!isAllowedActorLocation(actor)) {
			setFree(isBusy);
			return;
		}

        if (!isContainerAllowed(container)) {
			setFree(isBusy);
			return;
		}

		if (!isTriggerAllowed(container)) {
			setFree(isBusy);
			return;
		}

		const auto calendar = RE::Calendar::GetSingleton();
		auto now = calendar->GetDaysPassed();

        auto chance = getTakeLootChance(actor, container, elements, elementsCount);

		if (chance.size() <= 0) {
			setFree(isBusy);
			return;
        }
        
        bool trigger = false;

		for (const double c : chance) {
            if (c > 0) {
                if (c >= 1) {
					trigger = true;
					break;
				}
				if (randomChance(c)) {
					trigger = true;
					break;
				}
            }
		}

		if (!trigger) {
			incContainerChance(container);
			invalidateContainerData();
			setFree(isBusy);
			return;
        }

		_containerData[container->GetFormID()].lastTriggered = now;
		resetContainerChance(container);

        // Fire DEC
		DEBUG("onQLDoTaked Fire DEC");
		SKSE::ModCallbackEvent dTrapEvent{ "dtraps_TriggerTrap", "", 0, container };
		SKSE::GetModCallbackEventSource()->SendEvent(&dTrapEvent);

		invalidateContainerData();

		setFree(isBusy);
	}

	void InterfaceDeviouslyEnchantedChests::onQLDoSelect(RE::Actor* actor, SelectElement* elements, std::size_t elementsCount, RE::TESObjectREFR* container)
	{
        if (!QuickLootDD::Config::visualiseChance) {
			return;
        }

		if (!isReady.load(std::memory_order::relaxed)) {
			return;
		}

		if (!setBusy(isBusy)) {
			UI::Close();
			return;
		}

		if (!isAllowedActor(actor)) {
			UI::Close();
			setFree(isBusy);
			return;
		}

		if (!isAllowedActorLocation(actor)) {
			UI::Close();
			setFree(isBusy);
			return;
		}

		if (!isContainerAllowed(container)) {
			UI::Close();
			setFree(isBusy);
			return;
		}

		UIInfoData infoData;

		if (!isTriggerAllowed(container)) {
			infoData.isCooldown = true;
			UI::ShowDECInfo(infoData);
			setFree(isBusy);
			return;
		}


		auto chance = getSelectLootChance(actor, container, elements, elementsCount, &infoData);
		setFree(isBusy);
		if (chance.size() > 0) {
			UI::ShowDECInfo(infoData);
		}
	}

	void InterfaceDeviouslyEnchantedChests::onQLDoOpened(RE::Actor* , RE::TESObjectREFR* )
	{
	}

	void InterfaceDeviouslyEnchantedChests::onQLDoClosed(RE::Actor* , RE::TESObjectREFR* )
	{
		UI::Close();
	}

	void InterfaceDeviouslyEnchantedChests::onQLDoInvalidated(RE::Actor* , RE::TESObjectREFR* container, LootMenuElement* , std::size_t elementsCount)
	{
		if (!isReady.load(std::memory_order::relaxed)) {
			DEBUG("onQLDoInvalidated not loaded");
			return;
		}
		if (!container) {
			UI::Close();
			DEBUG("onQLDoInvalidated container is nullptr");
			return;
		}
        if (elementsCount <= 0) {
			UI::Close();
        }

		if (elementsCount <= 0 && QuickLootDD::Config::resetChanceOnLastItem) {
			if (!setBusy(isBusy)) {
				DEBUG("onQLDoInvalidated is busy");
				return;
			}
			if (auto search = _containerData.find(container->GetFormID()); search != _containerData.end() && search->second.chance != 0) {
				resetContainerChance(container);
            }
			setFree(isBusy);
        }

	}

	bool InterfaceDeviouslyEnchantedChests::isTriggerAllowed(RE::TESObjectREFR* container)
	{
		const auto calendar = RE::Calendar::GetSingleton();
		auto now = calendar->GetDaysPassed();

		for (auto it = _containerData.begin(); it != _containerData.end(); ++it) {
			if (it->first == container->GetFormID() && (now * 24 * 60 * 60) < ((it->second.lastTriggered * 24 * 60 * 60) + (QuickLootDD::Config::containerTriggerCooldown))) {
				//DEBUG("isTriggerAllowed container cooldown");
				return false;
			}
		}

		return true;
	}

    std::vector<double> InterfaceDeviouslyEnchantedChests::getTakeLootChance(RE::Actor* actor, RE::TESObjectREFR* container, TakenElement* element, std::size_t elementsCount)
	{
		std::vector<double> chances;

		auto containerChance = getContainerChance(container);
		auto locationChance = getLocationChance(actor);

		for (std::size_t i = 0; i < elementsCount; ++i) {
			chances.push_back(getItemChance(containerChance * locationChance, element[i].object, element[i].count));
		}
		return chances;
	}

	std::vector<double> InterfaceDeviouslyEnchantedChests::getSelectLootChance(RE::Actor* actor, RE::TESObjectREFR* container, SelectElement* element, std::size_t elementsCount, UIInfoData* infoData)
	{
		std::vector<double> chances;

		auto containerChance = getContainerChance(container);
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

	double InterfaceDeviouslyEnchantedChests::getItemChance(const double baseChance, RE::TESForm* object, std::size_t elementsCount, UIItemInfoData* itemInfoData)
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

	double InterfaceDeviouslyEnchantedChests::getContainerChance(RE::TESObjectREFR* container)
	{
		const auto calendar = RE::Calendar::GetSingleton();
		auto now = calendar->GetDaysPassed();
		bool found = false;

		for (auto it = _containerData.begin(); it != _containerData.end(); ++it) {
			if (it->first == container->GetFormID()) {
				found = true;
				if (it->second.chance == 0 || (now * 24 * 60 * 60) > ((it->second.lastUsed * 24 * 60 * 60) + (QuickLootDD::Config::containerChanceCooldown))) {
					break;
                }
				it->second.lastUsed = now;
				return it->second.chance;
			}
		}
        if (!found) {
			const auto [it, success] = _containerData.insert({ container->GetFormID(), { 0.0, 0.0f, 0.0f } });
        }
		return generateContainerChance(container);
	}

	void InterfaceDeviouslyEnchantedChests::incContainerChance(RE::TESObjectREFR* container)
	{
		double containerChance = getContainerChance(container) * randomValue(QuickLootDD::Config::increaseChanceMultiplierMin, QuickLootDD::Config::increaseChanceMultiplierMax);
		if (containerChance > QuickLootDD::Config::chanceLimit) {
			containerChance = QuickLootDD::Config::chanceLimit;
        }
		_containerData[container->GetFormID()].chance = containerChance;
		//DEBUG("incContainerChance for {:08X}: {}", container->GetFormID(), _containerData[container->GetFormID()].chance);
		_containerData[container->GetFormID()].lastUsed = RE::Calendar::GetSingleton()->GetDaysPassed();
	}

	void InterfaceDeviouslyEnchantedChests::decContainerChance(RE::TESObjectREFR* container)
	{
		double containerChance = getContainerChance(container) / randomValue(QuickLootDD::Config::increaseChanceMultiplierMin, QuickLootDD::Config::increaseChanceMultiplierMax);
		if (containerChance < 0) {
			containerChance = 0;
		}
		_containerData[container->GetFormID()].chance = containerChance;
		//DEBUG("decContainerChance for {:08X}: {}", container->GetFormID(), _containerData[container->GetFormID()].chance);
		_containerData[container->GetFormID()].lastUsed = RE::Calendar::GetSingleton()->GetDaysPassed();
	}

	double InterfaceDeviouslyEnchantedChests::resetContainerChance(RE::TESObjectREFR* container)
	{
		_containerData[container->GetFormID()].chance = 0;
		//DEBUG("resetContainerChance for {:08X}: {}", container->GetFormID(), _containerData[container->GetFormID()].chance);
		return _containerData[container->GetFormID()].chance;
	}

	double InterfaceDeviouslyEnchantedChests::generateContainerChance(RE::TESObjectREFR* container)
	{
		_containerData[container->GetFormID()].chance = randomValue(QuickLootDD::Config::baseChanceMin, QuickLootDD::Config::baseChanceMax);
		if (_containerData[container->GetFormID()].chance > QuickLootDD::Config::chanceLimit) {
			_containerData[container->GetFormID()].chance = QuickLootDD::Config::chanceLimit;
		}
		_containerData[container->GetFormID()].lastUsed = RE::Calendar::GetSingleton()->GetDaysPassed();
		//DEBUG("generateContainerChance for {:08X}: {}", container->GetFormID(), _containerData[container->GetFormID()].chance);
		return _containerData[container->GetFormID()].chance;
	}

	void InterfaceDeviouslyEnchantedChests::invalidateContainerData()
	{
		const auto calendar = RE::Calendar::GetSingleton();
		auto now = calendar->GetDaysPassed();

        if (now < _lastInvalidationTime + 1) {
			return;
        }

		auto now_sec = now * 24 * 60 * 60;

        struct SortedData
        {
			RE::FormID formID;
			float time;
        };

        std::vector<SortedData> sortedVector;

		DEBUG("invalidateContainerData pre:");
		for (auto it = _containerData.cbegin(); it != _containerData.cend();) {
			DEBUG("    {:08X}: {}, {}, {}", it->first, it->second.lastTriggered, it->second.lastUsed, it->second.chance);
			++it;
		}
		DEBUG("---------");

        std::erase_if(_containerData, [=, &sortedVector](const auto& pair) {
			bool toRemove = false;
			if (pair.second.lastTriggered == 0 || now_sec > ((pair.second.lastTriggered * 24 * 60 * 60) + (QuickLootDD::Config::containerTriggerCooldown))) {
				toRemove = true;
			}
			if (toRemove) {
				if (pair.second.chance == 0 || now_sec > ((pair.second.lastUsed * 24 * 60 * 60) + (QuickLootDD::Config::containerChanceCooldown))) {
					toRemove = true;
				} else {
					toRemove = false;
				}
			}
            if (!toRemove) {
				sortedVector.push_back({ pair.first, std::max(pair.second.lastTriggered, pair.second.lastUsed) });
            }
			return toRemove;
        });

        if (_containerData.size() > QuickLootDD::Config::containerLimit) {
			std::sort(sortedVector.begin(), sortedVector.end(),
				[](SortedData const& a, SortedData const& b) { return a.time < b.time; });

            auto cntRemove = _containerData.size() - QuickLootDD::Config::containerLimit;
			for (size_t i = 0; i < cntRemove; ++i) {
				_containerData.erase(sortedVector[i].formID);
            }
        }

        _lastInvalidationTime = now;

		DEBUG("invalidateContainerData post:");
		for (auto it = _containerData.cbegin(); it != _containerData.cend();) {
			DEBUG("    {:08X}: {}, {}, {}", it->first, it->second.lastTriggered, it->second.lastUsed, it->second.chance);
			++it;
		}
		DEBUG("---------");
	}

	double InterfaceDeviouslyEnchantedChests::getLocationChance(RE::Actor* actor)
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

	/*
    * TODO: not used
    */
	/*
    inline std::vector<RE::Actor*> InterfaceDeviouslyEnchantedChests::getNearestFollowers(RE::Actor* actor)
	{
		auto ret = QuickLootDD::TESUtils::getNearestActorsInRangeByFilter(actor, 426, [&](RE::Actor* a_actor) {
			if ((a_actor->IsPlayerTeammate() ||
				a_actor->IsInFaction(CurrentFollowerFaction) ||
					a_actor->IsInFaction(CurrentHireling)) &&
				a_actor->GetActorBase()->GetSex() == RE::SEX::kFemale) {
				return true;
			}
			return false;
		});
		return ret;
	}
    */

	inline bool InterfaceDeviouslyEnchantedChests::isAllowedActor(RE::Actor* actor)
	{
		return actor == RE::PlayerCharacter::GetSingleton() && actor->HasKeywordString(KeywordId.ActorTypeNPC);
	}

	inline bool InterfaceDeviouslyEnchantedChests::isAllowedActorLocation(RE::Actor* actor)
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

	bool InterfaceDeviouslyEnchantedChests::isContainerAllowed(RE::TESObjectREFR* container)
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

		if (QuickLootDD::Config::useDECContainerList && dt_containerformlist != nullptr && !dt_containerformlist->HasForm(baseObjContainer)) {
			DEBUG("isContainerAllowed dt_containerformlist not have <{:08X}:{}>", baseObjContainer->GetFormID(), baseObjContainer->GetName());
			return false;
		}
		return true;
	}
}