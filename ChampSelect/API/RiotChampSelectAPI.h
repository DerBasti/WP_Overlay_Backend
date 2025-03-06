#ifndef __LEAGUE_RIOT_CHAMP_SELECT_API__
#define __LEAGUE_RIOT_CHAMP_SELECT_API__

#include "../../Common/Riot/RiotLobbyClientAPI.h"
#include "../../Common/Data/CURLWrapper.h"
#include <unordered_map>
#include "../../Common/Data/json/document.h"
#include "../../Common/Data/Team.h"
#include "RiotChampSelectSnapshot.h"
#include "../../Common/Riot/RiotSummonerAPI.h"
#include "../../Common/Riot/RiotRankTiers.h"
#include "../../Common/Riot/RiotRankedStatsAndMasteryAPI.h"
#include "./ChampSelectSummonerActionType.h"
#include "./RiotChampSelectSummoner.h"

class SummonerSpell;

struct CurrentSummonerPick {
	uint32_t champId;
	bool currentlyActing;
	bool notFinishedPicking;
};

class ChampSelectTeam {
private:
	std::unordered_map<uint32_t, std::shared_ptr<ChampSelectSummoner>> member;
	TeamType teamType;
public:
	constexpr static uint8_t DEFAULT_AMOUNT_SLOTS = 5;
	ChampSelectTeam(TeamType type) {
		this->teamType = type;
	}
	virtual ~ChampSelectTeam() {

	}
	__inline void addMember(uint32_t pickingSlot, std::shared_ptr<ChampSelectSummoner> summoner) {
		this->member.insert(std::make_pair(pickingSlot, summoner));
	}
	__inline std::shared_ptr<ChampSelectSummoner> getMemberAt(uint32_t pickingSlot) const {
		return member.at(pickingSlot);
	}
	__inline std::shared_ptr<ChampSelectSummoner> getMemberWithSlotId(uint32_t slotId) const {
		for (auto it : member) {
			if (it.second->getSlotId() == slotId) {
				return it.second;
			}
		}
		return std::shared_ptr<ChampSelectSummoner>();
	}
	__inline const std::unordered_map<uint32_t, std::shared_ptr<ChampSelectSummoner>>& getAllMember() const {
		return member;
	}
	__inline uint32_t getMemberAmount() const {
		return static_cast<uint32_t>(member.size());
	}
	__inline bool updateMemberAt(uint8_t slot, const CurrentSummonerPick& pick) {
		auto currentMember = member.at(slot);
		bool updateFound = false;
		if (currentMember) {
			//updateFound = currentMember->getChampionId() != pick.champId && currentMember->isFinishedPicking() != pick.finishedPicking;
			updateFound = true;
			if (pick.currentlyActing) {
				currentMember->setCurrentlyActing();
			}
			currentMember->setChampionId(pick.champId);
			currentMember->setFinishedPicking(!pick.notFinishedPicking);
		}
		return updateFound;
	}
	__inline TeamType getTeamType() const {
		return teamType;
	}
};

class ChampSelectOverview {
private:
public:
	static const std::unordered_map<uint32_t, std::vector<uint32_t>> REGULAR_DRAFT_TURN_ORDER;
	static const std::unordered_map<uint32_t, std::vector<uint32_t>> TOURNAMENT_DRAFT_TURN_ORDER;
	ChampSelectOverview() { }
	virtual ~ChampSelectOverview() {}
};

enum class ClientWorkflowStatus : uint8_t {
	NO_CLIENT_DETECTED,
	CLIENT_FOUND,
	API_RUNNING,
	CHAMPSELECT_STARTED,
	CHAMPSELECT_RUNNING,
	ABORTED,
	FINISHED,
	AWAITING_POSTGAME,
	UNKNOWN = 0xFF
};

class RiotChampSelectAPI : public RiotLobbyClientAPI {
private:
	RiotSummonerAPI summonerApi;
	ChampSelectTeam* blueTeam;
	ChampSelectTeam* redTeam;
	std::shared_ptr<char> lastChampSelectSnapshotResponseJson;
	static constexpr const uint32_t DEFAULT_MAXIMUM_ALLOWED_SNAPSHOT_JSON_SIZE = 1024 * 100;
protected:
	std::vector<std::shared_ptr<ChampSelectSummoner>> extractSummonersFromTeamJson(rapidjson::Value::Array& teamObject, TeamType teamType);
	std::shared_ptr<ChampSelectSummoner> createSummonerByPickingSlot(uint8_t slot);
	CurrentSummonerPick getSelectedChampByPickingSlot(uint8_t slot);
	static const char *GET_SUMMONER_BY_SLOT_URI;
public:
	RiotChampSelectAPI(const wchar_t* leagueDirectory);
	virtual ~RiotChampSelectAPI();

	bool isChampSelectRunning();
	void updateTeamRoster();
	void clearTeamRoster();
	virtual std::unordered_map<TeamType, std::vector<std::shared_ptr<ChampSelectSummoner>>> getTeams();
	virtual std::shared_ptr<RiotChampSelectSnapshot> getChampSelectSnapshot();

	ClientWorkflowStatus detectChampselectWorkflowStatus();

	__inline TeamType getTeamTypeForSlotId(uint32_t slotId) const {
		return blueTeam->getMemberWithSlotId(slotId) ? TeamType::BLUE : TeamType::RED;
	}
	__inline ChampSelectTeam* getBlueTeam() const {
		return blueTeam;
	}
	__inline ChampSelectTeam* getRedTeam() const {
		return redTeam;
	}
	inline ChampSelectTeam* getTeamByTeamType(TeamType type) {
		return type == TeamType::BLUE ? getBlueTeam() : getRedTeam();
	}
};

#endif //__LEAGUE_RIOT_CHAMP_SELECT_API__