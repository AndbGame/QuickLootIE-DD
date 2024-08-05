#pragma once

namespace QuickLootDD::Utils
{

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

	void forEachActorsInRange(RE::Actor* target, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);

	std::vector<RE::Actor*> getNearestActorsInRangeByFilter(RE::Actor* actor, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);
}