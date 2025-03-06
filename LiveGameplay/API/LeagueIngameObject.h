#ifndef __LEAGUE_INGAME_OBJECT__
#define __LEAGUE_INGAME_OBJECT__
#pragma once

#include <inttypes.h>
#include <string>
#include <map>
#include "../../Common/Riot/DragonTypes.h"
#include "RiotMemoryReaderOffsets.h"
#include "LeagueIngameObjectType.h"

enum class ChampionSpellSlot : uint8_t {
	Q_SLOT,
	W_SLOT,
	E_SLOT,
	R_SLOT,
	D_SLOT,
	F_SLOT,
	SPELL_SLOT_AMOUNT
};

class RiotMemoryReader;

class LeagueIngameObject {
private:
	uint32_t networkId;
	int16_t objectIdx;
	int16_t teamId;
	float position[3];
	float currentHealth;
	float maxHealth;
	float currentMana;
	float maxMana;
	std::string internalName;
	std::string displayName;
	LeagueIngameObjectType objectType;
protected:
	LeagueIngameObject(uint32_t networkId, const std::pair<std::string, std::string>& internalAndDisplayStrings, LeagueIngameObjectType type);
public:
	LeagueIngameObject(const LeagueIngameObject& other) = delete;
	LeagueIngameObject(RiotMemoryReader* memoryReader, LeagueIngameObjectType type);
	virtual ~LeagueIngameObject();
	virtual void updateHullData(RiotMemoryReader* memoryReader);

	static std::string GetNameFromOffset(RiotMemoryReader* reader, RiotMemoryReaderOffsetType nameOffsetType);

	inline uint32_t getNetworkId() const {
		return networkId;
	}
	inline int16_t getObjectIndexId() const {
		return objectIdx;
	}
	inline int16_t getTeamId() const {
		return teamId;
	}
	inline float* getPosition() const {
		return (float*)position;
	}
	inline bool isAlive() const {
		return currentHealth < 0.0f;
	}
	inline float getCurrentHealth() const {
		return currentHealth;
	}
	inline float getMaxHealth() const {
		return maxHealth;
	}
	inline float getCurrentMana() const {
		return currentMana;
	}
	inline float getMaxMana() const {
		return maxMana;
	}
	inline const std::string& getInternalName() const {
		return internalName;
	}
	inline const std::string& getDisplayedName() const {
		return displayName;
	}
	inline LeagueIngameObjectType getObjectType() const {
		return objectType;
	}
};

class ChampionIngameObject : public LeagueIngameObject {
private:
	float experience;
	float currentGold;
	int16_t level;
	float totalGold;
	std::map<ChampionSpellSlot, float> spellCooldowns;
public:
	ChampionIngameObject(RiotMemoryReader* reader);
	virtual ~ChampionIngameObject();
	virtual void updateHullData(RiotMemoryReader* memoryReader);
	void updateSpellCooldowns(RiotMemoryReader* reader);

	inline float getExperience() const {
		return experience;
	}
	inline float getCurrentGold() const {
		return currentGold;
	}
	inline float getTotalGold() const {
		return totalGold;
	}
	inline float getCooldownFinished(ChampionSpellSlot requestedSpell) const {
		auto spellIt = spellCooldowns.find(requestedSpell);
		return spellIt != spellCooldowns.cend() ? spellIt->second : 0.0f;
	}
};


class NextDragonIndicatorIngameObject : public LeagueIngameObject {
private:
	DragonType dragonType;

	NextDragonIndicatorIngameObject(uint32_t netId, const std::pair<std::string, std::string>& names);
public:
	static NextDragonIndicatorIngameObject* FromStrings(uint32_t netId, const std::pair<std::string, std::string>& names);
	virtual ~NextDragonIndicatorIngameObject();

	inline DragonType getDragonType() const {
		return dragonType;
	}
};

class DragonIngameObject : public LeagueIngameObject {
private:
	DragonType dragonType;
public:
	DragonIngameObject(RiotMemoryReader* reader);
	virtual ~DragonIngameObject();

	inline DragonType getDragonType() const {
		return dragonType;
	}
};

#endif 