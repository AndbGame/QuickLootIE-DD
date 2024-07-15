#pragma once

namespace QuickLootDD::TESUtils
{
	void forEachActorsInRange(RE::Actor* target, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);

	std::vector<RE::Actor*> getNearestActorsInRangeByFilter(RE::Actor* actor, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);
}