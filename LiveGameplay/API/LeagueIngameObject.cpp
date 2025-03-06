#include "LeagueIngameObject.h"
#include "RiotMemoryReader.h"

LeagueIngameObject::LeagueIngameObject(RiotMemoryReader* memoryReader, LeagueIngameObjectType type) {
	updateHullData(memoryReader);

	internalName = GetNameFromOffset(memoryReader, RiotMemoryReaderOffsetType::OBJECT_NAME);
	displayName = GetNameFromOffset(memoryReader, RiotMemoryReaderOffsetType::OBJECT_DISPLAY_NAME);

	objectType = type;
}

LeagueIngameObject::LeagueIngameObject(uint32_t netId, const std::pair<std::string, std::string>& internalAndDisplayStrings, LeagueIngameObjectType type) {
	this->displayName = internalAndDisplayStrings.first;
	this->internalName = internalAndDisplayStrings.second;

	objectIdx = 0;
	teamId = 0;
	memset(position, 0x00, sizeof(float) * 3);
	currentHealth = maxHealth = currentMana = maxMana = 0.0f;

	objectType = type;
	this->networkId = netId;
}

LeagueIngameObject::~LeagueIngameObject() {

}
void LeagueIngameObject::updateHullData(RiotMemoryReader* memoryReader) {
	auto objectBuffer = memoryReader->getCurrentObjectBuffer();
	auto& offsets = memoryReader->getMemoryOffsets();
	memcpy(&networkId, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_NETWORK_ID)], sizeof(DWORD));
	memcpy(&objectIdx, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_INDEX)], sizeof(short));
	memcpy(&teamId, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_TEAM)], sizeof(short));
	memcpy(&position, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_POSITION)], sizeof(float) * 3);
	memcpy(&currentHealth, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_CURRENT_HEALTH)], sizeof(float));
	memcpy(&maxHealth, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_MAX_HEALTH)], sizeof(float));
	memcpy(&currentMana, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_CURRENT_MANA)], sizeof(float));
	memcpy(&maxMana, &objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_MAX_MANA)], sizeof(float));
}

std::string LeagueIngameObject::GetNameFromOffset(RiotMemoryReader* reader, RiotMemoryReaderOffsetType nameOffsetType) {
	auto objectBuffer = reader->getCurrentObjectBuffer();
	auto nameOffset = reader->getMemoryOffsets().getMemoryOffsetForOffsetType(nameOffsetType);
	int nameLength = *(int*)&objectBuffer[nameOffset + 0x10];
	std::string resultString;
	if (nameLength < 0 || nameLength > 50) {
		return resultString;
	}
	else if (nameLength < 16) {
		char buffer[0x10] = { 0x00 };
		strncpy_s(buffer, (const char*)&objectBuffer[nameOffset], nameLength);
		resultString = std::string(buffer);
	}
	else {
		char buffer[51];
		reader->readAsArray(*(uint64_t*)&objectBuffer[nameOffset], 50, (uint8_t*)buffer);
		resultString = std::string(buffer);
	}
	return resultString;
}


ChampionIngameObject::ChampionIngameObject(RiotMemoryReader* reader) : LeagueIngameObject(reader, LeagueIngameObjectType::CHAMPION) {
	updateHullData(reader);
}
ChampionIngameObject::~ChampionIngameObject() {

}

void ChampionIngameObject::updateHullData(RiotMemoryReader* reader) {
	LeagueIngameObject::updateHullData(reader);
	auto objectBuffer = reader->getCurrentObjectBuffer();
	auto& offsets = reader->getMemoryOffsets();
	experience = *((float*)&objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_CURRENT_EXPERIENCE)]);
	currentGold = *((float*)&objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_CURRENT_GOLD)]);
	level = *((int16_t*)&objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_LEVEL)]);
	totalGold = *((float*)&objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_TOTAL_GOLD)]);
	updateSpellCooldowns(reader);
}

void ChampionIngameObject::updateSpellCooldowns(RiotMemoryReader* reader) {
	auto objectBuffer = reader->getCurrentObjectBuffer();
	auto& offsets = reader->getMemoryOffsets();
	for (uint32_t i = 0; i < (uint32_t)ChampionSpellSlot::SPELL_SLOT_AMOUNT; i++) {
		uint64_t spellPointerAddress = *(uint64_t*)&objectBuffer[offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_SPELLBOOK_MANAGER) + offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_SPELLBOOK_SPELL_INFO) + (i * sizeof(uint64_t))];
		spellCooldowns.insert_or_assign((ChampionSpellSlot)i, reader->readAsObject<float>(spellPointerAddress + offsets.getMemoryOffsetForOffsetType(RiotMemoryReaderOffsetType::OBJECT_SPELL_DATA_COOLDOWN)));
	}
}

NextDragonIndicatorIngameObject::NextDragonIndicatorIngameObject(uint32_t netId, const std::pair<std::string, std::string>& names) : LeagueIngameObject(netId, names, LeagueIngameObjectType::NEXT_DRAGON_INDICATOR) {
	auto dragonSubName = getDisplayedName().substr(17);
	dragonSubName = dragonSubName.substr(0, dragonSubName.find('.'));
	std::string dragonTypeName = std::string("SRU_Dragon_") + dragonSubName;
	dragonType = DragonTypeStringified::FromString(dragonTypeName.c_str());
}

NextDragonIndicatorIngameObject* NextDragonIndicatorIngameObject::FromStrings(uint32_t netId, const std::pair<std::string, std::string>& names) {
	return new NextDragonIndicatorIngameObject(netId, names);
}

NextDragonIndicatorIngameObject::~NextDragonIndicatorIngameObject() {

}

DragonIngameObject::DragonIngameObject(RiotMemoryReader* reader) : LeagueIngameObject(reader, LeagueIngameObjectType::DRAGON) {
	dragonType = DragonTypeStringified::FromString(getDisplayedName().c_str());
}

DragonIngameObject::~DragonIngameObject() {

}