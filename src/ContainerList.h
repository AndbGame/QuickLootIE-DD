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

    // TODO
	class ContainerList : SpinLock
	{
	public:
		ContainerData getData(RE::FormID formId)
		{
			return _containerData[formId];
        }
	protected:
		friend class InterfaceDeviouslyEnchantedChests; // TODO: temporary
		std::map<RE::FormID, ContainerData> _containerData = {};
	};
}