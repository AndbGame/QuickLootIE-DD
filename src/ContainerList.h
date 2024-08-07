#pragma once

#include "Spinlock.h"

namespace QuickLootDD
{
	struct ContainerData
	{
		double chance = 0.0;
		float lastTriggered = 0.0f;
		float lastUsed = 0.0f;
	};

	class ContainerList : SpinLock
	{
	public:
		ContainerList();
		~ContainerList();

		void RevertState(SKSE::SerializationInterface* serializationInterface);
		void SaveState(SKSE::SerializationInterface* serializationInterface);
		void LoadState(SKSE::SerializationInterface* serializationInterface);

		bool getContainerData(RE::TESForm* form, ContainerData* data, bool onlyTry = false);
		void setContainerChance(RE::TESForm* form, double chance, bool onlyTry = false);
		void updateLastUsed(RE::TESForm* form, bool onlyTry = false);
		void setTriggered(RE::TESForm* form, bool onlyTry = false);
		float getLastTriggered(bool onlyTry = false);

		void Invalidate(bool force = false);

	protected:
		std::map<RE::FormID, ContainerData> _containerData = {};
		static inline float lastTriggered = 0.0f;
		float _lastInvalidationTime = 0;
	};
}