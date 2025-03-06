#ifndef __RIOT_CHAMP_SELECT_SUMMONER__
#define __RIOT_CHAMP_SELECT_SUMMONER__
#pragma once
class SummonerSpell;

#include "./ChampSelectSummonerActionType.h"
#include "../../Common/Riot/RiotSummoner.h"
#include "../../Common/Riot/RiotRankedStatsAndMasteryAPI.h"
#include "../../Common/Data/Team.h"
#include <inttypes.h>
#include <memory>
#include <vector>
#include <string>

class ChampSelectSummoner {
private:
	uint8_t slotId;
	uint64_t summonerId;
	uint32_t currentlyPickedChampionId;
	ChampSelectSummonerActionType currentAction;
	TeamType teamType;
	std::vector<SummonerSpell*> summonerSpells;
	bool finishedPicking;
	bool wasActing;
	std::string puuid;
	std::wstring name;
	RiotRanking rank;
#ifdef __LOL_CAREER_API_ENABLED__
	RiotChampionRankedStats championStats;
#endif
	std::shared_ptr<RiotMatchPlayerDataOverview> matchHistory;
public:
	ChampSelectSummoner(std::wstring botName, uint8_t slotId);
	ChampSelectSummoner(std::shared_ptr<RiotSummoner> summoner, uint8_t slotId);
	virtual ~ChampSelectSummoner();

	__inline uint64_t getSummonerId() const {
		return summonerId;
	}
	inline std::string getPuuid() const {
		return puuid;
	}
	__inline uint8_t getSlotId() const {
		return slotId;
	}
	__inline std::wstring getName() const {
		return name;
	}
	__inline TeamType getTeamType() const {
		return teamType;
	}
	__inline void setTeamType(TeamType type) {
		this->teamType = type;
	}
	__inline uint32_t getChampionId() const {
		return currentlyPickedChampionId;
	}
	__inline void setChampionId(uint32_t newId) {
		this->currentlyPickedChampionId = newId;
	}
	__inline ChampSelectSummonerActionType getCurrentAction() const {
		return currentAction;
	}
	__inline void setCurrentAction(ChampSelectSummonerActionType type) {
		this->currentAction = type;
	}
	__inline bool isFinishedPicking() const {
		return finishedPicking;
	}
	__inline bool isActingTurnDone() const {
		return getCurrentAction() == ChampSelectSummonerActionType::IDLE;
	}
	__inline void setCurrentlyActing() {
		this->wasActing = true;
	}
	__inline void setFinishedPicking(bool flag) {
		this->finishedPicking = flag;
	}
	inline RiotRanking getRank() const {
		return rank;
	}
	inline void setRank(RiotRanking newRank) {
		this->rank = newRank;
	}
	__inline const std::vector<SummonerSpell*>& getSummonerSpells() const {
		return summonerSpells;
	}
	__inline void setSummonerSpells(std::vector<SummonerSpell*> spells) {
		this->summonerSpells.clear();
		summonerSpells = spells;
	}
	inline std::shared_ptr<RiotMatchPlayerDataOverview> getMatchHistoryData() const {
		return matchHistory;
	}
	inline void setMatchHistoryData(std::shared_ptr<RiotMatchPlayerDataOverview> overviewData) {
		matchHistory = overviewData;
	}
#ifdef __LOL_CAREER_API_ENABLED__
	inline RiotChampionRankedStats getStatsOnChampion() const {
		return championStats;
	}
	inline void setStatsOnChampion(RiotChampionRankedStats newStats) {
		championStats = newStats;
	}
#endif
};

std::ostream& operator<<(std::ostream& out, const ChampSelectSummoner& summoner);
std::wostream& operator<<(std::wostream& out, const ChampSelectSummoner& summoner);

#endif