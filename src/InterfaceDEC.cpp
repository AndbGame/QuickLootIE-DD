#include "InterfaceDEC.h"

#include "TESUtils.h"

namespace QuickLootDD
{
    using VM = RE::BSScript::Internal::VirtualMachine;
    using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
    using CallbackPtr = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>;
    using TakedItem = QuickLootIE::QuickLootIEInterface::TakedItem;
    using clock = std::chrono::high_resolution_clock;

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

	inline bool setBusy(std::atomic<bool>& isBusy)
	{
		auto expected = false;
		return isBusy.compare_exchange_strong(expected, true);
	}

	inline void setFree(std::atomic<bool>& isBusy)
	{
		isBusy.store(false);
	}

	InterfaceDeviouslyEnchantedChests::InterfaceDeviouslyEnchantedChests()
	{
	}

	bool InterfaceDeviouslyEnchantedChests::Init()
	{
		return true;
	}
	bool InterfaceDeviouslyEnchantedChests::LoadForms()
	{
		RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

#define LOAD_FORM(NAME, TYPE, ID, PLUGIN)                                                                  \
	NAME = handler->LookupForm<TYPE>(ID, PLUGIN);                                                          \
	if (NAME == nullptr) {                                                                                 \
		ERROR("LoadForms : Not found <" #TYPE " - " #ID "> '" #NAME "' in '" PLUGIN "'"); \
		return false;                                                                                       \
	}

		LOAD_FORM(dtraps_Quest, RE::TESQuest, 0x000D62, "dD Enchanted Chests.esp");
		LOAD_FORM(dt_containerformlist, RE::BGSListForm, 0x001829, "dD Enchanted Chests.esp");
		LOAD_FORM(CurrentFollowerFaction, RE::TESFaction, 0x05C84E, "Skyrim.esm");
		LOAD_FORM(CurrentHireling, RE::TESFaction, 0x0BD738, "Skyrim.esm");

#undef LOAD_FORM
		isReady.store(true);
		return true;
	}

	bool InterfaceDeviouslyEnchantedChests::Reset()
	{
		_lastTriggered = clock::time_point::min();
		_lastLootedContainers.clear();
		isBusy.store(false);
		return true;
	}

	void QuickLootDD::InterfaceDeviouslyEnchantedChests::onQLDoTaked(RE::Actor* actor, std::vector<TakedItem> items, RE::TESObjectREFR* container, RE::TESForm* , bool )
	{
		if (!isReady.load(std::memory_order::relaxed) || actor != RE::PlayerCharacter::GetSingleton()) {
			DEBUG("onQLDoTaked not loaded");
			return;
		}
		if (!isAllowedActor(actor)) {
			DEBUG("onQLDoTaked Actor not allowed");
			return;
		}

		if (actor->GetActorBase()->GetSex() != RE::SEX::kFemale) {
			DEBUG("onQLDoTaked not female");
			return;
        }

        if (!isAllowedContainer(container)) {
			DEBUG("onQLDoTaked not allowed container <{:08X}:{}>", container ? container->GetFormID() : 0, container ? container->GetName() : "");
			return;
        }

		if (!setBusy(isBusy)) {
			DEBUG("onQLDoTaked is busy");
			return;
		}

		bool found = false;
		auto now = clock::now();
		if (now < (_lastTriggered + 5s)) {
			DEBUG("onQLDoTaked 5s cooldown");
			setFree(isBusy);
			return;
        }
		DEBUG("onQLDoTaked check _lastLootedContainers");
        for (auto it = _lastLootedContainers.begin(); it != _lastLootedContainers.end(); ) {
			if (now > (it->second + 5min)) {
                if (it->first == container->GetFormID()) {
					found = true;
					it->second = now;
					++it;
                } else {
					it = _lastLootedContainers.erase(it);
				}
			} else {
				if (it->first == container->GetFormID()) {
					DEBUG("onQLDoTaked container cooldown");
					setFree(isBusy);
					return;
                }
				++it;
			}
		}
        if (!found) {
			_lastLootedContainers[container->GetFormID()] = now;
        }

        // Fire DEC
        auto vm = VM::GetSingleton();
		auto policy = vm->GetObjectHandlePolicy();
		auto handle = policy->GetHandleForObject(dtraps_Quest->GetFormType(), dtraps_Quest);
		ObjectPtr object = nullptr;
		if (vm->FindBoundObject(handle, "dtraps_evaluateTrap", object)) {
			CallbackPtr callback(new EmptyRequestCallback("dtraps_evaluateTrap"));
			std::vector<RE::Actor*> followers = getNearestFollowers(actor);
			int dt_origintype = 2;
			bool isEmpty = false;

			auto args = RE::MakeFunctionArguments(std::forward<int>(dt_origintype), std::forward<RE::TESObjectREFR*>(container), std::forward<bool>(isEmpty), std::forward<std::vector<RE::Actor*>>(followers));

			DEBUG("onQLDoTaked dt_evaluate called");
			vm->DispatchMethodCall(object, "dt_evaluate", args, callback);
		} else {
			DEBUG("onQLDoTaked script dtraps_evaluateTrap not found");
        }

		DEBUG("_lastLootedContainers:");
		for (auto it = _lastLootedContainers.cbegin(); it != _lastLootedContainers.cend();) {
			DEBUG("    {:08X}: {}", it->first, it->second.time_since_epoch().count());
			++it;
		}
		DEBUG("---------");

		setFree(isBusy);
	}

	inline std::vector<RE::Actor*> InterfaceDeviouslyEnchantedChests::getNearestFollowers(RE::Actor* actor)
	{
		auto ret = QuickLootDD::TESUtils::getNearestActorsInRangeByFilter(actor, 0, [&](RE::Actor* a_actor) {
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

	inline bool InterfaceDeviouslyEnchantedChests::isAllowedContainer(RE::TESObjectREFR* container)
	{
		return dt_containerformlist != nullptr && container != nullptr && dt_containerformlist->HasForm(container);
	}

	inline bool InterfaceDeviouslyEnchantedChests::isAllowedActor(RE::Actor* actor)
	{
		return actor == RE::PlayerCharacter::GetSingleton() && actor->HasKeywordString(KeywordId.ActorTypeNPC);
	}
}