#pragma once

namespace QuickLootDD
{
    class Serialization
	{
	public:
		enum : std::uint32_t
		{
			sVersion = 1,

			sManager = 'mng',
			sContainerList = 'cnt',
		};

		struct ContainerData_v1
		{
			RE::FormID formId = 0;
			double chance = 0.0;
			float lastTriggered = 0.0f;
			float lastUsed = 0.0f;
		};

		static bool Init();
        
        static void LoadCallback(SKSE::SerializationInterface* serializationInterface);
		static void SaveCallback(SKSE::SerializationInterface* serializationInterface);
		static void RevertCallback(SKSE::SerializationInterface* serializationInterface);
    };
}