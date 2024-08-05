#include "Utils.h"

namespace QuickLootDD::Utils
{
	void forEachActorsInRange(RE::Actor* target, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback)
	{
		const auto position = target->GetPosition();

		for (auto actorHandle : RE::ProcessLists::GetSingleton()->highActorHandles) {
			if (auto actorPtr = actorHandle.get()) {
				if (auto actor = actorPtr.get()) {
					if (a_range == 0 || position.GetDistance(actor->GetPosition()) <= a_range) {
						auto loc_refBase = actor->GetActorBase();
						if (actor && actor != target && !actor->IsDisabled() && actor->Is3DLoaded() &&
							!actor->IsPlayerRef() && !actor->IsDead() &&
							(actor->Is(RE::FormType::NPC) || (loc_refBase && loc_refBase->Is(RE::FormType::NPC))) &&
							!actor->IsGhost() && !actor->IsOnMount()) {
							if (!a_callback(actor)) {
								break;
							}
						}
					}
				}
			}
		}
	}

	std::vector<RE::Actor*> getNearestActorsInRangeByFilter(RE::Actor* actor, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback)
	{
		std::vector<RE::Actor*> ret;
		forEachActorsInRange(actor, a_range, [&](RE::Actor* a_actor) {
			if (a_callback(a_actor)) {
				ret.push_back(a_actor);
			}
			return true;
		});
		return ret;
	}
}