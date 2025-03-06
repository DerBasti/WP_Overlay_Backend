#ifndef __LEAGUE_CHAMP_SELECT_OVERVIEW_SAVE_STATE__
#define __LEAGUE_CHAMP_SELECT_OVERVIEW_SAVE_STATE__

#include <string>
#include <inttypes.h>
#include <unordered_map>
#include <vector>
#include "Team.h"
#include "LanePositions.h"
#include "../Logging/Logger.h"

class ChampSelectSummoner;

class SummonerSaveState {
private:
	std::wstring summonerName;
	TeamType teamType;
	SummonerLanePosition position;
	uint32_t championId;
	uint32_t firstSummonerSpellId;
	uint32_t secondSummonerSpellId;
public:
	SummonerSaveState(ChampSelectSummoner* summoner);
	SummonerSaveState(std::wstring summonerName, TeamType type, SummonerLanePosition pos, uint32_t champId, uint32_t firstSummoner, uint32_t secondSummoner) {
		this->summonerName = summonerName;
		this->position = pos;
		this->teamType = type;
		championId = champId;
		firstSummonerSpellId = firstSummoner;
		secondSummonerSpellId = secondSummoner;
	}
	virtual ~SummonerSaveState() {

	}
	__inline TeamType getTeamType() const {
		return teamType;
	}
	__inline std::wstring getSummonerName() const {
		return summonerName;
	}
	__inline uint32_t getChampionId() const {
		return championId;
	}
	__inline uint32_t getFirstSummonerSpellId() const {
		return firstSummonerSpellId;
	}
	__inline uint32_t getSecondSummonerSpellId() const {
		return secondSummonerSpellId;
	}
	__inline SummonerLanePosition getLanePosition() const {
		return position;
	}
};

class SummonerSaveStateCollector {
private:
	std::vector<SummonerSaveState> collectedSaveStates;
	ROSEThreadedLogger logger;
	uint64_t gameId;
	uint64_t saveTimestamp;
	const static std::wstring DEFAULT_FILE_PATH;
	void loadNextSaveState(FILE* fh);
	void saveNextSaveState(FILE* fh, const SummonerSaveState& state) const;
public:
	SummonerSaveStateCollector() {
		gameId = -1;
		saveTimestamp = 0;
	}
	virtual ~SummonerSaveStateCollector() {

	}

	bool loadStates();
	bool saveStates() const;
	void clearStates() {
		collectedSaveStates.clear();
	}

	__inline void addState(SummonerSaveState&& saveState) {
		collectedSaveStates.push_back(std::move(saveState));
	}

	inline uint64_t getGameId() const {
		return gameId;
	}
	inline void setGameId(uint64_t gameId) {
		this->gameId = gameId;
	}
	inline uint64_t getSaveTimestamp() const {
		return saveTimestamp;
	}
	__inline const std::vector<SummonerSaveState>& getSavedStates() const {
		return collectedSaveStates;
	}
};

std::wostream& operator<<(std::wostream& out, const SummonerSaveState& saveState);
std::ostream& operator<<(std::ostream& out, const SummonerSaveState& saveState);

#endif //__LEAGUE_CHAMP_SELECT_OVERVIEW_SAVE_STATE__