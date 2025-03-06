#ifndef __RIOT_SUMMONER_API__
#define __RIOT_SUMMONER_API__

#include "RiotLobbyClientAPI.h"
#include "RiotSummoner.h"
#include "ChampMastery.h"
#include <map>

class RiotSummonerAPI : public RiotLobbyClientAPI {
private:
	static const char *GET_SUMMONERNAME_BY_SUMMONER_ID_URI;
	static const char *GET_SUMMONERNAME_BY_SUMMONER_PUUID_URI;
	static constexpr const char *GET_CURRENTLY_LOGGED_IN_SUMMONER = "lol-summoner/v1/current-summoner";
	static constexpr const char *GET_SUMMONER_RANK_BY_PUUID_URI = "lol-ranked/v1/ranked-stats/%s";
	static constexpr const char *GET_SUMMONER_BY_NAME = "lol-summoner/v1/summoners?name=%s";
	static constexpr const char *GET_TOP5_PLAYED_CHAMPS_FOR_SUMMONER_BY_SUMMONER_ID = "lol-champion-mastery/v1/%s/champion-mastery/top?limit=5";
	static constexpr const char *GET_ALL_CHAMP_MASTERY_FOR_SUMMONER_BY_SUMMONER_ID = "lol-champion-mastery/v1/%s/champion-mastery";
	static constexpr const char *SOLO_QUEUE_QUEUE_TYPE_NAME = "RANKED_SOLO_5x5";
protected:
	std::shared_ptr<RiotSummoner> extractSummonerFromJson(rapidjson::Document& json);
	RiotRanking extractRankingFromJson(rapidjson::Document& json);
	RankedTier GetTierFromString(const char* str);
	std::unordered_map<uint32_t, PlayerChampionMastery> extractChampMasteriesFromJson(rapidjson::Document& json, std::string summonerPuuid);
	std::unordered_map<std::string, std::shared_ptr<RiotSummoner>> summonerByPuuid;
	std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> extractChampMasteriesSortedByAmountFromJson(rapidjson::Document& json, std::string summonerPuuid);
public:
	RiotSummonerAPI(const wchar_t* leagueDirectory);
	virtual ~RiotSummonerAPI();

	std::shared_ptr<RiotSummoner> getCurrentlyLoggedInSummoner();
	std::shared_ptr<RiotSummoner> findSummonerBySummonerId(uint64_t accountId, bool isRepeatedRequest=false);
	std::shared_ptr<RiotSummoner> findSummonerByPuuid(std::string puuid);
	std::shared_ptr<RiotSummoner> findSummonerByName(std::string name);
	RiotRanking findRankingForSummonerByPuuid(std::string puuid);
	std::unordered_map<uint32_t, PlayerChampionMastery> getChampMasteriesForSummonerId(std::string puuid);
	std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> getChampMasteriesForSummonerSortedByPoints(std::string summonerPuuid);
};

#endif //__RIOT_SUMMONER_API__