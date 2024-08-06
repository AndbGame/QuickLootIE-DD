#include "ContainerList.h"

#include "Config.h"
#include "Serialization.h"

namespace QuickLootDD
{
	ContainerList::ContainerList()
	{
	}
	ContainerList::~ContainerList()
	{
	}
	void ContainerList::RevertState(SKSE::SerializationInterface* /*serializationInterface*/)
	{
		UniqueSpinLock lock(*this);
		_containerData.clear();
		_lastInvalidationTime = 0.0;
	}
	void ContainerList::SaveState(SKSE::SerializationInterface* serializationInterface)
	{
		UniqueSpinLock lock(*this);

		Invalidate(true);

		const std::size_t cntData = _containerData.size();
		if (!serializationInterface->WriteRecordData(cntData)) {
			ERROR("Failed to save count of data ({})", cntData);
			return;
		}
		for (auto it = _containerData.cbegin(); it != _containerData.cend(); ++it) {
			Serialization::ContainerData_v1 data;
			data.formId = it->first;
			data.lastUsed = it->second.lastUsed;
			data.lastTriggered = it->second.lastTriggered;
			data.chance = it->second.chance;

			if (!serializationInterface->WriteRecordData(data)) {
				ERROR("Failed to save data for <{:X}>", data.formId);
			}
		}
		if (!serializationInterface->WriteRecordData(lastTriggered)) {
			ERROR("Failed to save lastTriggered data");
		}
	}
	void ContainerList::LoadState(SKSE::SerializationInterface* serializationInterface)
	{
		UniqueSpinLock lock(*this);
		std::size_t cntData;
		serializationInterface->ReadRecordData(cntData);

		_containerData.clear();

		Serialization::ContainerData_v1 data;

		for (size_t i = 0; i < cntData; i++) {
			serializationInterface->ReadRecordData(data);
			if (!data.formId) {
				continue;
			}
			_containerData[data.formId].lastUsed = data.lastUsed;
			_containerData[data.formId].lastTriggered = data.lastTriggered;
			_containerData[data.formId].chance = data.chance;
			TRACE("Loaded data for <{:08X}>: ({}, {}, {})", data.formId, data.lastTriggered, data.lastUsed, data.chance);
		}
		serializationInterface->ReadRecordData(lastTriggered);

		_lastInvalidationTime = RE::Calendar::GetSingleton()->GetDaysPassed();
	}
	bool ContainerList::getContainerData(RE::TESForm* form, ContainerData* data, bool onlyTry)
	{
        if (onlyTry) {
            if (!trySpinLock()) {
				return false;
            }
		} else {
			spinLock();
		}

        *data = _containerData[form->GetFormID()];

        spinUnlock();
		return true;
	}
	void ContainerList::setContainerChance(RE::TESForm* form, double chance, bool onlyTry)
	{
		if (onlyTry) {
			if (!trySpinLock()) {
				return;
			}
		} else {
			spinLock();
		}

		_containerData[form->GetFormID()].chance = chance;
		_containerData[form->GetFormID()].lastUsed = RE::Calendar::GetSingleton()->GetDaysPassed();

		spinUnlock();
	}
	void ContainerList::setTriggered(RE::TESForm* form, bool onlyTry)
	{
		if (onlyTry) {
			if (!trySpinLock()) {
				return;
			}
		} else {
			spinLock();
		}

		lastTriggered = RE::Calendar::GetSingleton()->GetDaysPassed();
		_containerData[form->GetFormID()].lastTriggered = lastTriggered;

		spinUnlock();
	}
	float ContainerList::getLastTriggered(bool onlyTry)
	{
		if (onlyTry) {
			if (!trySpinLock()) {
				return 0.0f;
			}
		} else {
			spinLock();
		}

		auto ret = lastTriggered;

		spinUnlock();
		return ret;
	}
	void ContainerList::Invalidate(bool force)
	{
		UniqueSpinLock lock(*this);

		auto now = RE::Calendar::GetSingleton()->GetDaysPassed();

		if (!force && !(now > _lastInvalidationTime + 1 || _containerData.size() >= QuickLootDD::Config::containerMaxLimitForInvalidate)) {
			return;
		}

		auto now_sec = now * 24 * 60 * 60;

		struct SortedData
		{
			RE::FormID formID;
			float time;
		};

		std::vector<SortedData> sortedVector;

		DEBUG("invalidateContainerData pre:");
		for (auto it = _containerData.cbegin(); it != _containerData.cend();) {
			DEBUG("    <{:08X}>: ({}, {}, {})", it->first, it->second.lastTriggered, it->second.lastUsed, it->second.chance);
			++it;
		}
		DEBUG("---------");

		std::erase_if(_containerData, [=, &sortedVector](const auto& pair) {
			bool toRemove = false;
			if (pair.second.lastTriggered == 0 || now_sec > ((pair.second.lastTriggered * 24 * 60 * 60) + (QuickLootDD::Config::containerTriggerCooldown))) {
				toRemove = true;
			}
			if (toRemove) {
				if (pair.second.chance == 0 || now_sec > ((pair.second.lastUsed * 24 * 60 * 60) + (QuickLootDD::Config::containerChanceCooldown))) {
					toRemove = true;
				} else {
					toRemove = false;
				}
			}
			if (!toRemove) {
				sortedVector.push_back({ pair.first, std::max(pair.second.lastTriggered, pair.second.lastUsed) });
			}
			return toRemove;
		});

		if (_containerData.size() > QuickLootDD::Config::containerLimit) {
			std::sort(sortedVector.begin(), sortedVector.end(),
				[](SortedData const& a, SortedData const& b) { return a.time < b.time; });

			auto cntRemove = _containerData.size() - QuickLootDD::Config::containerLimit;
			for (size_t i = 0; i < cntRemove; ++i) {
				_containerData.erase(sortedVector[i].formID);
			}
		}

		_lastInvalidationTime = now;

		DEBUG("invalidateContainerData post:");
		for (auto it = _containerData.cbegin(); it != _containerData.cend();) {
			DEBUG("    <{:08X}>: ({}, {}, {})", it->first, it->second.lastTriggered, it->second.lastUsed, it->second.chance);
			++it;
		}
		DEBUG("---------");
	}
}