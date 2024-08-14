#pragma once

namespace QuickLootDD::Utils
{

	static inline bool randomChance(const double chance, const double min = 0, const double max = 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> distr(min, max);
		auto rnd = distr(gen);
		return rnd < chance;
	}
	
	static inline std::int32_t randomValue(const std::int32_t min = 0, const std::int32_t max = 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<std::int32_t> distr(min, max);

		return distr(gen);
	}

	static inline double randomValue(const double min = 0, const double max = 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> distr(min, max);

		return distr(gen);
	}

    constexpr int GetMaskForSlot(uint32_t slot)
	{
		if (slot < 29 || slot > 61)
			return 0;

		return (1 << (slot - 30));
	}

	static inline RE::VMHandle GetHandle(const RE::TESForm* a_form)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm->GetObjectHandlePolicy();
		return policy->GetHandleForObject(a_form->GetFormType(), a_form);
	}

	static inline RE::BSTSmartPointer<RE::BSScript::Object> GetScriptObject(const RE::TESForm* a_form, std::string a_class, bool a_create = false)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto handle = GetHandle(a_form);

		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		bool found = vm->FindBoundObject(handle, a_class.c_str(), object);
		if (!found && a_create) {
			vm->CreateObject2(a_class, object);
			vm->BindObject(object, handle, false);
		}

		return object;
	}

	class WornVisitor : public RE::InventoryChanges::IItemChangeVisitor
	{
	public:
		WornVisitor(std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> a_fun) :
			_fun(a_fun){};
		~WornVisitor() = default;

		virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) override
		{
			return _fun(a_entryData);
		}

	private:
		std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> _fun;
	};

	void forEachActorsInRange(RE::Actor* target, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);

	std::vector<RE::Actor*> getNearestActorsInRangeByFilter(RE::Actor* actor, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);
}