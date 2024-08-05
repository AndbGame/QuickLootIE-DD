#include "Serialization.h"

#include "Manager.h"

namespace QuickLootDD
{
	bool Serialization::Init()
	{
		TRACE("Init CoSave");
		auto* serializationInterface = SKSE::GetSerializationInterface();
		serializationInterface->SetUniqueID(_byteswap_ulong('QLDD'));
		serializationInterface->SetSaveCallback(Serialization::SaveCallback);
		serializationInterface->SetRevertCallback(Serialization::RevertCallback);
		serializationInterface->SetLoadCallback(Serialization::LoadCallback);
		TRACE("Init CoSave done.");
		return true;
	}
	void Serialization::LoadCallback(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Serialization::Load");
		uint32_t type;
		uint32_t version;
		uint32_t length;
		while (serializationInterface->GetNextRecordInfo(type, version, length)) {
			if (version != sVersion) {
				WARN("Invalid Version for loaded Data of Type = {}. Expected = {}; Got = {}", type, (int)sVersion, version);
				continue;
			}
			logger::info("Loading record {}", type);
			switch (type) {
			case sManager:
				Manager::LoadState(serializationInterface);
				break;
			default:
				break;
			}
        }
	}
	void Serialization::SaveCallback(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Serialization::Save");
		if (serializationInterface->OpenRecord(sManager, sVersion)) {
			Manager::SaveState(serializationInterface);
		} else {
			ERROR("Failed to open co-save record <InterfaceDeviouslyEnchantedChests> for write")
        }
	}
	void Serialization::RevertCallback(SKSE::SerializationInterface* serializationInterface)
	{
		TRACE("Serialization::Revert");
		Manager::RevertState(serializationInterface);
	}
}