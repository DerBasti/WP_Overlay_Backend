#include "RiotSummonerAPI.h"
#include "../Logging/ErrorCodeTranslator.h"

RiotSummonerAPI::RiotSummonerAPI(const wchar_t* leagueDirectory) : RiotLobbyClientAPI(leagueDirectory) {

}

RiotSummonerAPI::~RiotSummonerAPI() {

}

std::shared_ptr<RiotSummoner> RiotSummonerAPI::extractSummonerFromJson(rapidjson::Document& json) {
	const char* summonerName = json["gameName"].GetString();
	const char* tagLine = json["tagLine"].GetString();
	const char* puuid = json["puuid"].GetString();
	uint64_t accountId = json["accountId"].GetUint64();
	uint64_t summonerId = json["summonerId"].GetUint64();
	std::string summonerString = summonerName;
	if (_stricmp(tagLine, "EUW") != 0) {
		summonerString += std::string("#") + std::string(tagLine);
	}
	return std::shared_ptr<RiotSummoner>(new RiotSummoner(std::string(puuid), accountId, summonerId, summonerString));
}

std::shared_ptr<RiotSummoner> RiotSummonerAPI::findSummonerBySummonerId(uint64_t summonerId, bool isRepeatedRequest) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_SUMMONERNAME_BY_SUMMONER_ID_URI, summonerId);
#ifdef _DEBUG
	logger.logDebug("Requesting SummonerName for SummonerID: ", summonerId);
#endif
	auto doc = getRequestToApi(requestBuffer);
	std::shared_ptr<RiotSummoner> result;
	if (!doc.HasParseError() && doc.IsObject()) {
		result = extractSummonerFromJson(doc);
	}
	else {
		logger.logWarn("SummonerName came up empty. Possibly Botaccount?");
	}
	return result;
}

std::shared_ptr<RiotSummoner> RiotSummonerAPI::getCurrentlyLoggedInSummoner() {
	auto doc = getRequestToApi(GET_CURRENTLY_LOGGED_IN_SUMMONER);
	std::shared_ptr<RiotSummoner> result;
	if (!doc.HasParseError() && doc.IsObject()) {
		result = extractSummonerFromJson(doc);
	}
	else {
		logger.logWarn("SummonerName came up empty. Possibly Botaccount?");
	}
	return result;
}

/*

	for (auto c : name) {
		if (c >= 'A' && c <= 'z') {
			out << (char)c;
		}
		else if ((unsigned char)c >= 0x80 && (unsigned char)c <= 0xBF) {
			out << "%2C" << '%' << std::hex << std::setprecision(2) << std::setw(2)
				<< std::setfill('0') << (unsigned char)(static_cast<int>(c));

		}
		else if((unsigned char)c >= 0xC0) {
			out << "%3C" << '%' << std::hex << std::setprecision(2) << std::setw(2)
				<< std::setfill('0') << (unsigned char)(static_cast<int>(c))-0x30;
		}
		else {
			out << '%' << std::hex << std::setprecision(2) << std::setw(2)
				<< std::setfill('0') << (unsigned char)(static_cast<int>(c));
		}
	}
*/

std::shared_ptr<RiotSummoner> RiotSummonerAPI::findSummonerByName(std::string name) {
	char requestBuffer[0x300] = { 0x00 };
	std::string replacedName;
	replacedName.reserve(name.length() + 1);
	if (name.find('#') == std::string::npos) {
		name += std::string("#EUW");
	}
	std::ostringstream out;
	for (auto c : name) {
		if ((c >= '0' && c <= '9') || (toupper(c) >= 'A' && toupper(c) <= 'Z')) {
			out << c;
		}
		else {
			out << '%' << std::setw(2) << std::setfill('0') << std::hex << (static_cast<int>(c) & 0xFF);
		}
	}
	replacedName = out.str();
	sprintf_s(requestBuffer, GET_SUMMONER_BY_NAME, replacedName.c_str());
#ifdef _DEBUG
	logger.logDebug("Requesting Summoner for SummonerName: ", name.c_str());
#endif
	auto doc = getRequestToApi(requestBuffer);
	std::shared_ptr<RiotSummoner> result;
	if (!doc.HasParseError() && doc.IsObject()) {
		result = extractSummonerFromJson(doc);
	}
	else {
		logger.logWarn("SummonerName came up empty. Possibly Botaccount?");
	}
	return result;
}

std::shared_ptr<RiotSummoner> RiotSummonerAPI::findSummonerByPuuid(std::string puuid) {
	auto summonerIt = summonerByPuuid.find(puuid);
	if (summonerIt != summonerByPuuid.cend()) {
		return summonerIt->second;
	}
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_SUMMONERNAME_BY_SUMMONER_PUUID_URI, puuid.c_str());

	logger.logDebug("Requesting SummonerName for Summoner-PUUID: ", puuid.c_str());

	auto doc = getRequestToApi(requestBuffer);
	std::shared_ptr<RiotSummoner> result;
	if (!doc.HasParseError() && doc.IsObject()) {
		result = extractSummonerFromJson(doc);
		summonerByPuuid.insert_or_assign(puuid, result);
	}
	else {
		logger.logWarn("SummonerName came up empty. Possibly Botaccount?");
	}
	return result;
}

RiotRanking RiotSummonerAPI::findRankingForSummonerByPuuid(std::string puuid) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_SUMMONER_RANK_BY_PUUID_URI, puuid.c_str());
	logger.logDebug("Requesting SummonerRank for Summoner-PUUID: ", puuid.c_str());

	auto doc = getRequestToApi(requestBuffer);
	RiotRanking result;
	if (!doc.HasParseError() && doc.IsObject()) {
		result = extractRankingFromJson(doc);
	}
	return result;
}

RiotRanking RiotSummonerAPI::extractRankingFromJson(rapidjson::Document& json) {
	auto soloQueueRankJson = json["queues"].GetArray();
	for (auto entry = soloQueueRankJson.Begin(); entry != soloQueueRankJson.End(); entry++) {
		auto entryObject = entry->GetObj();
		auto queueTypeString = entryObject["queueType"].GetString();
		if (_stricmp(queueTypeString, SOLO_QUEUE_QUEUE_TYPE_NAME) == 0) {
			RankedTier tier = GetTierFromString(entryObject["tier"].GetString());
			RiotRanking result(tier, RiotRanking::FromStringToDivision(entryObject["division"].GetString()), entryObject["leaguePoints"].GetInt());
			return result;
		}
	}
	return RiotRanking();
}

RankedTier RiotSummonerAPI::GetTierFromString(const char* str) {
	static std::unordered_map<std::string, RankedTier> RANKED_TIER{
		{"NONE", RankedTier::UNRANKED},
		{"BRONZE", RankedTier::BRONZE},
		{"SILVER", RankedTier::SILVER},
		{"GOLD", RankedTier::GOLD},
		{"PLATINUM", RankedTier::PLATINUM},
		{"EMERALD", RankedTier::EMERALD},
		{"DIAMOND", RankedTier::DIAMOND},
		{"MASTER", RankedTier::MASTER},
		{"GRANDMASTER", RankedTier::GRANDMASTER},
		{"CHALLENGER", RankedTier::CHALLENGER},
	};
	return RANKED_TIER.find(str) != RANKED_TIER.cend() ? RANKED_TIER.at(str) : RankedTier::UNRANKED;
}

std::unordered_map<uint32_t, PlayerChampionMastery> RiotSummonerAPI::getChampMasteriesForSummonerId(std::string puuid) {
	std::unordered_map<uint32_t, PlayerChampionMastery> result;
	if (puuid.empty()) {
		logger.logDebug("Querying (unsorted) ChampionMasteries for empty SummonerId (0). Returning empty result set by default.");
		return result;
	}
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_ALL_CHAMP_MASTERY_FOR_SUMMONER_BY_SUMMONER_ID, puuid.c_str());
	logger.logDebug("Querying (unsorted) ChampionMasteries for SummonerPUUID: ", puuid.c_str());
	auto doc = getRequestToApi(requestBuffer);
	if (!doc.HasParseError() && doc.IsArray()) {
		result = extractChampMasteriesFromJson(doc, puuid);
	}
	return result;
}

std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> RiotSummonerAPI::getChampMasteriesForSummonerSortedByPoints(std::string summonerPuuid) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_ALL_CHAMP_MASTERY_FOR_SUMMONER_BY_SUMMONER_ID, summonerPuuid.c_str());
	logger.logDebug("Querying (sorted by mastery points) ChampionMasteries for SummonerPUUID: ", summonerPuuid.c_str());
	auto doc = getRequestToApi(requestBuffer);
	std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> result;
	if (!doc.HasParseError() && doc.IsArray()) {
		result = extractChampMasteriesSortedByAmountFromJson(doc, summonerPuuid);
	}
	return result;
}

std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> RiotSummonerAPI::extractChampMasteriesSortedByAmountFromJson(rapidjson::Document& json, std::string summonerPuuid) {
	std::map<uint64_t, PlayerChampionMastery, std::greater<uint64_t>> result;
	for (auto it = json.Begin(); it != json.End(); it++) {
		auto currentChampion = it->GetObj();
		PlayerChampionMastery mastery(summonerPuuid, currentChampion["championId"].GetUint(), currentChampion["championPoints"].GetUint64());
		result.insert({ mastery.getMasteryAmount(), mastery });
	}
	return result;
}

std::unordered_map<uint32_t, PlayerChampionMastery> RiotSummonerAPI::extractChampMasteriesFromJson(rapidjson::Document& json, std::string summonerPuuid) {
	std::unordered_map<uint32_t, PlayerChampionMastery> result;
	for (auto it = json.Begin(); it != json.End(); it++) {
		auto currentChampion = it->GetObj();
		PlayerChampionMastery mastery(summonerPuuid, currentChampion["championId"].GetUint(), currentChampion["championPoints"].GetUint64());
		result.insert({ mastery.getChampionId(), mastery });
	}
	return result;
}

const char *RiotSummonerAPI::GET_SUMMONERNAME_BY_SUMMONER_ID_URI = "lol-summoner/v1/summoners/%lld";
const char *RiotSummonerAPI::GET_SUMMONERNAME_BY_SUMMONER_PUUID_URI = "lol-summoner/v2/summoners/puuid/%s";