#ifndef __LEAGUE_INGAME_EVENT_CACHE__
#define __LEAGUE_INGAME_EVENT_CACHE__

#pragma once

#include "../../Common/Riot/EpicMonsterTypes.h"
#include <unordered_map>

class LeagueIngameEventCache {
private:
	std::vector<float> dragonKillTimes;
	std::vector<float> riftHeraldKillTimes;
	std::vector<float> baronKillTimes;
	std::vector<float> invalidEpicKillTimes;
	inline std::vector<float>& getEpicKillTimeVector(EpicMonsterType monsterType) {
		switch (monsterType) {
		case EpicMonsterType::DRAGON:
			return dragonKillTimes;
		case EpicMonsterType::RIFT_HERALD:
			return riftHeraldKillTimes;
		case EpicMonsterType::BARON_NASHOR:
			return baronKillTimes;
		}
		return invalidEpicKillTimes;
	}
	inline const std::vector<float>& getEpicKillTimeVector(EpicMonsterType monsterType) const {
		switch (monsterType) {
		case EpicMonsterType::DRAGON:
			return dragonKillTimes;
		case EpicMonsterType::RIFT_HERALD:
			return riftHeraldKillTimes;
		case EpicMonsterType::BARON_NASHOR:
			return baronKillTimes;
		}
		return invalidEpicKillTimes;
	}
public:
	LeagueIngameEventCache();
	virtual ~LeagueIngameEventCache();

	inline bool addKillTiming(EpicMonsterType monsterType, float ingameTime) {
		return addKillTiming(monsterType, ingameTime, 300.0f);
	}
	bool addKillTiming(EpicMonsterType monsterType, float ingameTime, float respawnTimeBetweenKills);
	float getLastKillTime(EpicMonsterType monsterType, float currentIngameTime) const;
};

#endif //__LEAGUE_INGAME_EVENT_CONTROLLER__