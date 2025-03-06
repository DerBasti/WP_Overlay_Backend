#ifndef __RIOT_POST_MATCH_API__
#define __RIOT_POST_MATCH_API__

#include "RiotLobbyClientAPI.h"
#include "RiotPostMatch.h"

enum class RiotSpectateType : uint8_t {
	INVALID,
	MYSELF_INGAME,
	LIVE_SPECTATE,
	THIRDPARTY_SITE_SPECTATE,
	REPLAY_SPECTATE,
};

enum class RiotLocale : uint8_t {
	WESTERN,
	KOREAN
};

std::ostream& operator<<(std::ostream& out, const RiotSpectateType& slot);
std::wostream& operator<<(std::wostream& out, const RiotSpectateType& slot);

class RiotGameSpectateInfo {
private:
	uint64_t gameId;
	RiotSpectateType spectateType;
	RiotLocale ingameLocale;
public:
	static constexpr const uint64_t DEFAULT_INVALID_GAME_ID = (std::numeric_limits<uint64_t>::max)();
	constexpr RiotGameSpectateInfo() : RiotGameSpectateInfo(DEFAULT_INVALID_GAME_ID, RiotSpectateType::INVALID) {

	}
	constexpr RiotGameSpectateInfo(uint64_t gameId, RiotSpectateType type) : RiotGameSpectateInfo(gameId, type, RiotLocale::WESTERN) {
	}
	constexpr RiotGameSpectateInfo(uint64_t gameId, RiotSpectateType type, RiotLocale ingameLocale) : gameId(gameId), spectateType(type), ingameLocale(ingameLocale) {
	}
	virtual ~RiotGameSpectateInfo() {}

	inline bool operator==(const RiotGameSpectateInfo& other) const {
		return gameId == other.gameId && spectateType == other.spectateType;
	}
	inline bool operator!=(const RiotGameSpectateInfo& other) const {
		return !(operator==(other));
	}

	inline uint64_t getGameId() const {
		return gameId;
	}
	inline RiotSpectateType getSpectateType() const {
		return spectateType;
	}
	inline RiotLocale getIngameLocale() const {
		return ingameLocale;
	}
};

std::ostream& operator<<(std::ostream& out, const RiotGameSpectateInfo& gameInfo);
std::wostream& operator<<(std::wostream& out, const RiotGameSpectateInfo& gameInfo);

class RiotPostMatchAPI : public RiotLobbyClientAPI {
private:
	rapidjson::Document getPostMatchDataForSummonerPuuid(const std::string& summonerPuuid);
	static constexpr const char* POST_MATCH_QUERY_FOR_SUMMONER_PUUID = "lol-match-history/v1/products/lol/%s/matches?begIndex=2&endIndex=2";
	static constexpr const char* GET_GAME_ID_FROM_CURRENT_SPECTATE = "lol-gameflow/v1/session";
	static constexpr const char* POST_MATCH_QUERY_FOR_GAME_ID = "lol-match-history/v1/games/%lld";
	static constexpr const char* POST_MATCH_EVENTS_QUERY_FOR_GAME_ID = "lol-match-history/v1/game-timelines/%lld";

	std::pair<uint64_t, RiotSpectateType> getGameIdFromSpectateProcess();
	RiotLocale extractLocaleFromIngameProcess();
public:
	static const RiotGameSpectateInfo DEFAULT_INVALID_SPECTATE;
	RiotPostMatchAPI(const wchar_t* leagueDirectory);
	virtual ~RiotPostMatchAPI();

	RiotGameSpectateInfo getGameIdFromCurrentLiveSpectate(); 
	std::shared_ptr<RiotPostMatch> getPostMatchData(uint64_t gameId);
};

#endif //__RIOT_POST_MATCH_API__