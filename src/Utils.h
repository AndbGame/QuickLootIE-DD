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