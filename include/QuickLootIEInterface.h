#pragma once

/*
	Header File for QuickLootIE integration
*/

namespace QuickLootIE
{
	class QuickLootIEInterface
	{
	public:
		enum TakedSource
		{
			GROUNG = 1,
			CONTAINER = 2
		};
		struct TakedItem
		{
			RE::TESForm* object;
			std::int32_t count;
		};
		struct TakedEvent
		{
			RE::Actor* actor;
			TakedSource source;
			std::vector<TakedItem> items;
			RE::TESObjectREFR* container;
			RE::TESForm* containerOwner;
			bool isStealAlarm;
		};
		typedef void (*OnTakedHandler)(TakedEvent evt);

		/*
		* Public methods
		*/
		virtual uint16_t getMajorVersion() = 0;
		virtual uint16_t getMinorVersion() = 0;
		virtual bool registerOnTakedHandler(OnTakedHandler handler) = 0;
	};

	enum MessageType : std::uint32_t
	{
		Interface = 0xC0000003
	};

	struct InterfaceMessage
	{
		QuickLootIEInterface* quickLootInterface;
	};
}
