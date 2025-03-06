#include "RiotPostMatchAPI.h"
#include "../Data/ChampionReader.h"
#include "../ProcessWatcher.h"

const RiotGameSpectateInfo RiotPostMatchAPI::DEFAULT_INVALID_SPECTATE = RiotGameSpectateInfo(RiotGameSpectateInfo::DEFAULT_INVALID_GAME_ID, RiotSpectateType::INVALID);


RiotPostMatchAPI::RiotPostMatchAPI(const wchar_t* leagueDirectory) : RiotLobbyClientAPI(leagueDirectory) {

}

RiotPostMatchAPI::~RiotPostMatchAPI() {

}

RiotGameSpectateInfo RiotPostMatchAPI::getGameIdFromCurrentLiveSpectate() {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_GAME_ID_FROM_CURRENT_SPECTATE);
	auto doc = getRequestToApi(requestBuffer, false);
	uint64_t gameId = 0;
	RiotSpectateType spectateType = RiotSpectateType::INVALID;
	if (doc.IsNull() || doc.HasParseError() || doc["gameData"].GetObj()["gameId"].GetUint64() <= 0) {
		auto result = getGameIdFromSpectateProcess();
		gameId = result.first;
		spectateType = result.second;
	}
	else {
		bool isSelfPlaying = doc["gameClient"].GetObj()["serverIp"].GetStringLength() > 5;
		gameId = doc["gameData"].GetObj()["gameId"].GetUint64();
		spectateType = gameId == 0 ? RiotSpectateType::INVALID : (isSelfPlaying ? RiotSpectateType::MYSELF_INGAME : RiotSpectateType::LIVE_SPECTATE);
	}
	RiotLocale ingameLocale = extractLocaleFromIngameProcess();
	return RiotGameSpectateInfo(gameId, spectateType, ingameLocale);
}

std::pair<uint64_t, RiotSpectateType> RiotPostMatchAPI::getGameIdFromSpectateProcess() {
	Process process = ProcessWatcher::GetProcessWithStartParameters(L"League Of Legends.exe");
	if (!process.isValid()) {
		return std::pair<uint64_t, RiotSpectateType>();
	}
	uint64_t gameId = RiotGameSpectateInfo::DEFAULT_INVALID_GAME_ID;
	RiotSpectateType spectateType = RiotSpectateType::INVALID;
	for (auto param : process.getStartParameters()) {
		if (param.find(L"-GameID") != std::wstring::npos) {
			logger.logDebug("Found process parameter token 'spectator'. Parsing for gameId...");
			wchar_t* buffer;
			wchar_t* tokenizedParam = wcstok_s((wchar_t*)param.c_str(), L" ", &buffer);
			while (tokenizedParam != nullptr) {
				gameId = _wtoll(tokenizedParam);
				if (gameId != 0) {
					logger.logDebug("Thirdparty-GameId found: ", gameId);
					spectateType = RiotSpectateType::THIRDPARTY_SITE_SPECTATE;
					break;
				}
				tokenizedParam = wcstok_s(nullptr, L" ", &buffer);
			}
			if (gameId != 0) {
				break;
			}
			logger.logDebug("No Thirdparty-GameId found in token '", param.c_str(), "'. Continuing parsing.");
		}
		if (param.find(L".rofl") != std::wstring::npos) {
			logger.logDebug("Found process parameter for replays ('*.rofl'-file): ", param.c_str());
			std::wstring path = param.substr(param.find_last_of('/') + 1);
			path = path.substr(path.find('-') + 1);
			spectateType = RiotSpectateType::REPLAY_SPECTATE;
			gameId = _wtoll(path.c_str());
			break;
		}
	}
	logger.logDebug("Replay-GameId found: ", gameId);
	return std::make_pair(gameId, spectateType);
}

RiotLocale RiotPostMatchAPI::extractLocaleFromIngameProcess() {
	Process process = ProcessWatcher::GetProcessWithStartParameters(L"League Of Legends.exe");
	RiotLocale resultLocale = RiotLocale::WESTERN;
	if (!process.isValid()) {
		//logger.logWarn("Ingame process appears to be no longer available. Returning default ingame locale: WESTERN");
		return resultLocale;
	}
	uint64_t gameId = RiotGameSpectateInfo::DEFAULT_INVALID_GAME_ID;
	RiotSpectateType spectateType = RiotSpectateType::INVALID;
	for (auto param : process.getStartParameters()) {
		if (param.find(L"-Locale") != std::wstring::npos) {
			logger.logDebug("Locale found in program argument list: ", param.c_str());
			if (param.find(L"KR") != std::wstring::npos) {
				logger.logDebug("Korean ingame locale detected.");
				resultLocale = RiotLocale::KOREAN;
			}
			else {
				logger.logDebug("Using default locale: English/Western");
			}
			break;
		}
	}
	return resultLocale;
}

std::shared_ptr<RiotPostMatch> RiotPostMatchAPI::getPostMatchData(uint64_t gameId) {
	logger.logDebug("Requesting PostMatch data for gameId ", gameId);
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, POST_MATCH_QUERY_FOR_GAME_ID, gameId);
	auto doc = getRequestToApi(requestBuffer);
	std::shared_ptr<RiotPostMatch> result;
	if (doc.IsNull() || doc.HasParseError()) {
		logger.logDebug("Postmatch-Json for gameId ", gameId, " is empty or has a parse error : ", doc.GetParseError());
		return result;
	}
	sprintf_s(requestBuffer, POST_MATCH_EVENTS_QUERY_FOR_GAME_ID, gameId);
	auto eventDoc = getRequestToApi(requestBuffer);
	if (eventDoc.IsNull() || eventDoc.HasParseError()) {
		logger.logDebug("Postmatch-Event-JSON for GameId ", gameId, " is null. Returning empty Postmatch data.");
		return result;
	}
	RiotSummonerAPI summonerApi(this->getLeagueDirectory().c_str());
	RiotPostMatch* postMatch = new RiotPostMatch(doc, eventDoc, &summonerApi);
	result = std::shared_ptr<RiotPostMatch>(postMatch);
	return result;
}

rapidjson::Document RiotPostMatchAPI::getPostMatchDataForSummonerPuuid(const std::string& summonerPuuid) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, POST_MATCH_QUERY_FOR_SUMMONER_PUUID, summonerPuuid.c_str());
	logger.logDebug("Requesting last match history for SummonerPUUID: ", summonerPuuid.c_str());

	auto doc = getRequestToApi(requestBuffer);
	return doc;
}

std::ostream& operator<<(std::ostream& out, const RiotSpectateType& phase) {
	static const char* phaseNames[] = { "INVALID", "MYSELF_INGAME",
		"LIVE_SPECTATE", "THIRDPARTY_SITE_SPECTATE", "REPLAY_SPECTATE" };
	out << phaseNames[(int)phase];
	return out;
}
std::wostream& operator<<(std::wostream& out, const RiotSpectateType& phase) {
	static const char* phaseNames[] = { "INVALID", "MYSELF_INGAME",
		"LIVE_SPECTATE", "THIRDPARTY_SITE_SPECTATE", "REPLAY_SPECTATE" };
	out << phaseNames[(int)phase];
	return out;
}

std::ostream& operator<<(std::ostream& out, const RiotGameSpectateInfo& gameInfo) {
	out << "[GameId: " << gameInfo.getGameId() << ", SpectateType: " << gameInfo.getSpectateType() << "]";
	return out;
}

std::wostream& operator<<(std::wostream& out, const RiotGameSpectateInfo& gameInfo) {
	out << "[GameId: " << gameInfo.getGameId() << ", SpectateType: " << gameInfo.getSpectateType() << "]";
	return out;
}