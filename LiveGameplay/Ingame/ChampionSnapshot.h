#ifndef __LEAGUE_CHAMPION_SNAPSHOT__
#define __LEAGUE_CHAMPION_SNAPSHOT__

#include <unordered_map>
#include <array>
#include <string>

#include "../../Common/Data/json/document.h"
#include "../../Common/Data/Team.h"

class Item;
class SummonerSpell;
class Champion;

class IngamePlayerSnapshot {
private:
	uint8_t spectatorSlotId;
	Champion* champion;
	std::array<Item*, 7> inventory;
	std::array<bool, 7> itemActivatable;
	uint8_t level;
	std::wstring summonerName;
	TeamType team;
	uint32_t creepsKilled;
	uint32_t champsKilled;
	uint32_t deaths;
	uint32_t killAssists;
	float respawnTimer;
	SummonerSpell* firstSummonerSpell;
	SummonerSpell* secondSummonerSpell;
public:
	IngamePlayerSnapshot(rapidjson::GenericObject<false, rapidjson::Value>& obj, uint8_t spectatorSlotId);
	virtual ~IngamePlayerSnapshot();

	bool doesInventoryItemExist(uint32_t id) const;

	inline bool isItemNotActivable(uint32_t slotId) const {
		return itemActivatable[slotId];
	}
	inline const std::array<bool, 7>& getItemActivatableStatus() const {
		return itemActivatable;
	}
	__inline uint8_t getSpectatorSlotId() const {
		return spectatorSlotId;
	}
	__inline const std::array<Item*, 7>& getInventory() const {
		return inventory;
	}
	__inline uint8_t getLevel() const {
		return level;
	}
	__inline std::wstring getSummonerName() const {
		return summonerName;
	}
	__inline uint32_t getCreepsKilled() const {
		return creepsKilled;
	}
	__inline uint32_t getChampsKilled() const {
		return champsKilled;
	}
	__inline uint32_t getDeaths() const {
		return deaths;
	}
	__inline uint32_t getAssists() const {
		return killAssists;
	}
	__inline float getRespawnTimer() const {
		return respawnTimer;
	}
	__inline SummonerSpell* getFirstSummonerSpell() const {
		return firstSummonerSpell;
	}
	__inline SummonerSpell* getSecondSummonerSpell() const {
		return secondSummonerSpell;
	}
	Champion* getChampion() const {
		return champion;
	}
	std::string getChampionName() const;
};

class Team {
private:
	TeamType teamId;
	std::unordered_map<uint32_t, IngamePlayerSnapshot*> champions;
public:
	Team() : Team(TeamType::BLUE) {

	}
	Team(TeamType teamId) {
		this->teamId = teamId;
	}
	virtual ~Team() {
		for (auto champPair : champions) {
			delete champPair.second;
		}
	}

	__inline TeamType getTeamType() const {
		return teamId;
	}

	__inline void addChampionSnapshot(IngamePlayerSnapshot* champ) {
		this->champions.insert(std::make_pair(champ->getSpectatorSlotId(), champ));
	}

	__inline const std::unordered_map<uint32_t, IngamePlayerSnapshot*>& getChampions() const {
		return champions;
	}

	__inline IngamePlayerSnapshot* getChampionSnapshotById(uint32_t slot) {
		return champions.at(slot);
	}
	__inline size_t getPlayerAmount() const {
		return champions.size();
	}
};

#endif //__LEAGUE_CHAMPION_SNAPSHOT__