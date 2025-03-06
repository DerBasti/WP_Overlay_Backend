#ifndef __LEAGUE_SNAPSHOT_DELTA__
#define __LEAGUE_SNAPSHOT_DELTA__

#include "../API/RiotPlayerList.h"
#include "../API/RiotEventList.h"
#include "../../Common/Riot/DragonTypes.h"
#include <unordered_set>
#include <unordered_map>
#include <set>

class LevelupSnapshotDelta {
private:
	IngamePlayerSnapshot* champ;
	uint8_t previousLevel;
	uint8_t currentLevel;
public:
	LevelupSnapshotDelta(IngamePlayerSnapshot* currentChamp, uint8_t previousLvl, uint8_t currentLvl) {
		champ = currentChamp;
		this->previousLevel = previousLvl;
		this->currentLevel = currentLvl;
	}
	virtual ~LevelupSnapshotDelta() {

	}
	__inline IngamePlayerSnapshot* getChampSnapshot() const {
		return champ;
	}
	__inline uint8_t getPreviousLevel() const {
		return previousLevel;
	}
	__inline uint8_t getCurrentLevel() const {
		return currentLevel;
	}
};

class DragonKillHistory {
private:
	std::unordered_map<TeamType, uint32_t> drakesKilledPerTeam;
	std::unordered_map<float, std::pair<TeamType, DragonType>> dragonKillTimes;
	std::set<float> killTimepoints;
public:
	DragonKillHistory() {
		drakesKilledPerTeam.insert_or_assign(TeamType::BLUE, 0);
		drakesKilledPerTeam.insert_or_assign(TeamType::RED, 0);
	}
	virtual ~DragonKillHistory() {}

	void addDragonToTeam(TeamType team, DragonType type, float killTimepoint) {
		drakesKilledPerTeam.insert_or_assign(team, drakesKilledPerTeam.at(team) + 1);
		dragonKillTimes.insert_or_assign(killTimepoint, std::make_pair(team, type));
		killTimepoints.insert(killTimepoint);
	}
	inline float getLastKillTimepoint() const {
		return killTimepoints.empty() ? 0.0f : *killTimepoints.rbegin();
	}
	DragonType getDrakeSoulpointType() const {
		return dragonKillTimes.size() >= 3 && drakesKilledPerTeam.at(TeamType::BLUE) < 4 && drakesKilledPerTeam.at(TeamType::RED) < 4 ?
			dragonKillTimes.at(getLastKillTimepoint()).second : DragonType::UNKNOWN;
	}
	inline bool isSoulpointAcquired() const {
		return drakesKilledPerTeam.at(TeamType::BLUE) >= 4 || drakesKilledPerTeam.at(TeamType::RED) >= 4;
	}
};

class SnapshotDelta {
private:
	std::unordered_set<LevelupSnapshotDelta*> levelups;
	std::unordered_map<IngamePlayerSnapshot*, std::vector<Item*>> itemsFinished;
	std::unordered_map<IngamePlayerSnapshot*, float> creepsPerMinute;
	std::unordered_map<TeamType, uint32_t> teamKills;
	std::unordered_map<TeamType, uint32_t> turretKills;
	std::vector<InhibitorKilledEvent*> activelyRecoveringInhibs;
	std::vector<InhibitorRespawnedEvent*> respawnedInhibitors;
	std::unordered_map<EpicMonsterType, float> lastEpicKillTimes;
	DragonKillHistory dragonKillHistory;
	void handleChampsFromTeam(Team* teamPreviousSnapshot, Team* teamCurrentSnapshot, float gametime);
	float currentGameTime;
	bool gameStarted;
public:
	SnapshotDelta(RiotPlayerList* previous, RiotPlayerList* current, RiotEventList* previousEvents, RiotEventList* events);
	virtual ~SnapshotDelta();

	__inline const std::unordered_set<LevelupSnapshotDelta*>& getLevelups() const {
		return levelups;
	}
	__inline const std::unordered_map<IngamePlayerSnapshot*, std::vector<Item*>>& getItemsFinished() const {
		return itemsFinished;
	}
	__inline const std::vector<InhibitorKilledEvent*> getRecoveringInhibitors() const {
		return activelyRecoveringInhibs; 
	}
	__inline const std::vector<InhibitorRespawnedEvent*> getRespawnedInhibitors() const {
		return respawnedInhibitors;
	}
	__inline const std::unordered_map<IngamePlayerSnapshot*, float> getCreepsPerMinute() const {
		return creepsPerMinute;
	}
	__inline uint32_t getTurretsKilledByTeam(TeamType type) const {
		return turretKills.at(type);
	}
	__inline uint32_t getKillsByTeam(TeamType type) const {
		return teamKills.at(type);
	}
	__inline float getCurrentGameTime() const {
		return currentGameTime;
	}
	float getLastKillTime(EpicMonsterType monsterType) const {
		return lastEpicKillTimes.find(monsterType) != lastEpicKillTimes.cend() ? lastEpicKillTimes.at(monsterType) : (std::numeric_limits<float>::max)();
	}
	inline bool isCurrentDragonSoulpoint() const {
		auto soulpointType = dragonKillHistory.getDrakeSoulpointType();
		return soulpointType != DragonType::UNKNOWN && soulpointType != DragonType::ELDER;
	}
	inline DragonType getDragonSoulpointType() const {
		return dragonKillHistory.getDrakeSoulpointType();
	}
	inline bool isDragonSoulAcquired() const {
		return dragonKillHistory.isSoulpointAcquired();
	}
};

#endif //__LEAGUE_SNAPSHOT_DELTA__