#pragma once

#include "InterfaceDEC.h"

namespace QuickLootDD
{
	class InterfaceQuickLootIE
	{
    public:
		static bool Init(const SKSE::MessagingInterface* messagingInterface);

        static void setDEC(InterfaceDeviouslyEnchantedChests* dec) { _dec = dec; }
		static InterfaceDeviouslyEnchantedChests* getDEC() { return _dec; }

    protected:
		static InterfaceDeviouslyEnchantedChests* _dec;
    };
}