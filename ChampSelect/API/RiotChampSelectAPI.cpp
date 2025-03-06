#include "RiotChampSelectAPI.h"
#include "../../Common/Data/ChampionReader.h"
#include "../../Common/ProcessWatcher.h"
#include "RiotChampSelectSnapshot.h"
#include "../../Common/Riot/RiotIngameAPI.h"
#include "../../Common/Data/json/prettywriter.h"
#include "../../Common/Data/json/stringbuffer.h"
#include "../../Common/ProjectFilePathHandler.h"
#include "../../Common/Data/SummonerSpell.h"
#include "../../Common/Logging/ErrorCodeTranslator.h"

RiotChampSelectAPI::RiotChampSelectAPI(const wchar_t* leagueDirectory) : RiotLobbyClientAPI(leagueDirectory), summonerApi(leagueDirectory) {
	lastChampSelectSnapshotResponseJson = std::shared_ptr<char>(new char[DEFAULT_MAXIMUM_ALLOWED_SNAPSHOT_JSON_SIZE], std::default_delete<char[]>());
	blueTeam = nullptr;
	redTeam = nullptr;
}

RiotChampSelectAPI::~RiotChampSelectAPI() {
	delete blueTeam;
	blueTeam = nullptr;

	delete redTeam;
	redTeam = nullptr;
}


std::shared_ptr<ChampSelectSummoner> RiotChampSelectAPI::createSummonerByPickingSlot(uint8_t slot) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_SUMMONER_BY_SLOT_URI, slot);
	auto doc = getRequestToApi(requestBuffer);
	ChampSelectSummoner* summoner = nullptr;
	if (!doc.HasParseError() && doc.IsObject()) {
		uint64_t summonerId = doc["summonerId"].GetUint64();
		if (summonerId > 0) {
			summoner = new ChampSelectSummoner(summonerApi.findSummonerBySummonerId(summonerId), slot);
			logger.logInfo("Found valid summoner at slot ", (uint32_t)slot, ": ", *summoner);
		}
		else {
			logger.logWarn("Summoner at slot ", (uint32_t)slot, " doesn't have a SummonerId. Returning empty summoner.");
		}
	}
	else {
		logger.logWarn("Json-Response for Summoner at slot: ", (uint32_t)slot, " is invalid/errornous. Returning empty summoner.");
	}
	return std::shared_ptr<ChampSelectSummoner>(summoner);
}

CurrentSummonerPick RiotChampSelectAPI::getSelectedChampByPickingSlot(uint8_t slot) {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, GET_SUMMONER_BY_SLOT_URI, slot);
	auto doc = getRequestToApi(requestBuffer);

	CurrentSummonerPick pick = { 0,0 };
	if (!doc.HasParseError() && doc.IsObject()) {
		pick.champId = (uint32_t)doc["championId"].GetInt();
		pick.currentlyActing = doc["isActingNow"].GetBool();
		pick.notFinishedPicking = !doc["isDonePicking"].GetBool();
	}
	return pick;
}

bool RiotChampSelectAPI::isChampSelectRunning() {
	char requestBuffer[0x300] = { 0x00 };
	sprintf_s(requestBuffer, "lol-champ-select/v1/session");
	auto doc = getRequestToApi(requestBuffer, false);
	return !doc.HasParseError();
}

std::shared_ptr<RiotChampSelectSnapshot> RiotChampSelectAPI::getChampSelectSnapshot() {
	auto doc = getRequestToApi("lol-champ-select/v1/session");
	std::shared_ptr<RiotChampSelectSnapshot> snapshot;
	if (doc.IsObject() && doc.HasMember("actions")) {
#ifdef _DEBUG
		dumpDataIntoLog((ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"champ_select.json")).c_str());
#endif
		snapshot = std::shared_ptr<RiotChampSelectSnapshot>(new RiotChampSelectSnapshot(doc));
	}
	else {
		logger.logWarn("Error while getting champ select session data. Error:");
		logger.logWarn(RiotAPI::TransformJsonToString(&doc).c_str());
		logger.logWarn("---- End ChampSelect Session error.");
	}
	return snapshot;
}


std::vector<std::shared_ptr<ChampSelectSummoner>> RiotChampSelectAPI::extractSummonersFromTeamJson(rapidjson::Value::Array& teamObject, TeamType teamType) {
	std::vector<std::shared_ptr<ChampSelectSummoner>> teamResult;
	for (auto currentTeamMemberIt = teamObject.Begin(); currentTeamMemberIt != teamObject.End(); currentTeamMemberIt++) {
#ifdef _DEBUG
		RiotAPI::LogJsonObject(currentTeamMemberIt);
#endif
		uint32_t slotId = currentTeamMemberIt->GetObj()["cellId"].GetInt();
		uint64_t summonerId = currentTeamMemberIt->GetObj()["summonerId"].GetUint64();
		uint32_t championId = currentTeamMemberIt->GetObj()["championId"].GetInt();
		bool isBot = summonerId == 0 && championId != 0;
		ChampSelectSummoner* summoner = nullptr;
		if (isBot) {
			auto champion = ChampionDatabase::getInstance()->getChampionById(championId);
			summoner = new ChampSelectSummoner(champion->getIngameNameUnicode(), slotId);
		}
		else {
			summoner = new ChampSelectSummoner(summonerApi.findSummonerBySummonerId(summonerId), slotId);
		}
		//myTeamType = static_cast<TeamType>(currentTeamMemberIt->GetObj()["team"].GetInt()-1);
		std::shared_ptr<ChampSelectSummoner> wrappedSummoner(summoner);
		wrappedSummoner->setChampionId(championId);
		if (isBot) {
			wrappedSummoner->setFinishedPicking(true);
		}
		wrappedSummoner->setTeamType(teamType);
		teamResult.push_back(wrappedSummoner);
	}
	return teamResult;
}

std::unordered_map<TeamType, std::vector<std::shared_ptr<ChampSelectSummoner>>> RiotChampSelectAPI::getTeams() {
	auto doc = getRequestToApi("lol-champ-select/v1/session");
	if (doc.HasParseError()) {
		logger.logWarn("Error while requesting SummonerTeam: ", doc.GetParseError());
		return std::unordered_map<TeamType, std::vector<std::shared_ptr<ChampSelectSummoner>>>();
	}
	std::unordered_map<TeamType, std::vector<std::shared_ptr<ChampSelectSummoner>>> teams;
	std::vector<std::shared_ptr<ChampSelectSummoner>> myTeam;
	std::vector<std::shared_ptr<ChampSelectSummoner>> theirTeam;

	auto myTeamJson = doc["myTeam"].GetArray();
	TeamType myTeamType = TeamType::BLUE;

	myTeam = extractSummonersFromTeamJson(myTeamJson, TeamType::BLUE);
	if (doc.HasMember("theirTeam")) {
		auto theirTeamJson = doc["theirTeam"].GetArray();
		theirTeam = extractSummonersFromTeamJson(theirTeamJson, TeamType::RED);
	}
	teams.insert_or_assign(TeamType::BLUE, std::move(myTeam));
	teams.insert_or_assign(TeamType::RED, std::move(theirTeam));
	return teams;
}

void RiotChampSelectAPI::updateTeamRoster() {
	if (blueTeam == nullptr && redTeam == nullptr) {
		logger.logDebug("Team rosters are empty. Loading full team information.");
		auto teams = getTeams();
		if (teams.empty()) {
			return;
		}
		auto blueTeamSummoners = teams.at(TeamType::BLUE);
		auto redTeamSummoners = teams.at(TeamType::RED);

		std::function<void(const std::vector<std::shared_ptr<ChampSelectSummoner>>&, ChampSelectTeam*)> addTeamMember = [&](const std::vector<std::shared_ptr<ChampSelectSummoner>>& summoners, ChampSelectTeam* team) {
			for (uint32_t i = 0; i < summoners.size(); i++) {
				auto currentSummoner = summoners.at(i);
				auto rank = summonerApi.findRankingForSummonerByPuuid(currentSummoner->getPuuid());
				currentSummoner->setRank(std::move(rank));
				team->addMember(i, currentSummoner);
			}
		};

		blueTeam = new ChampSelectTeam(TeamType::BLUE);
		redTeam = new ChampSelectTeam(TeamType::RED);

		addTeamMember(blueTeamSummoners, blueTeam);
		addTeamMember(redTeamSummoners, redTeam);
	}
	else {
		for (uint32_t i = 0;blueTeam && i < blueTeam->getMemberAmount(); i++) {
			auto summoner = blueTeam->getMemberAt(i);
			blueTeam->updateMemberAt(i, getSelectedChampByPickingSlot(summoner->getSlotId()));
		}
		for (uint32_t i = 0; redTeam && i < redTeam->getMemberAmount(); i++) {
			auto summoner = redTeam->getMemberAt(i);
			redTeam->updateMemberAt(i, getSelectedChampByPickingSlot(summoner->getSlotId()));
		}
	}
}

void RiotChampSelectAPI::clearTeamRoster() {
	delete blueTeam;
	blueTeam = nullptr;

	delete redTeam;
	redTeam = nullptr;
}

ClientWorkflowStatus RiotChampSelectAPI::detectChampselectWorkflowStatus() {
	Process leagueProcess = ProcessWatcher::GetProcess(L"LeagueClientUx.exe");
	if (!leagueProcess.isValid()) {
		return ClientWorkflowStatus::NO_CLIENT_DETECTED;
	}
	if (!isAvailable()) {
		return ClientWorkflowStatus::CLIENT_FOUND;
	}
	bool champSelectRunningFlag = isChampSelectRunning();
	if (champSelectRunningFlag) {
		return ClientWorkflowStatus::CHAMPSELECT_RUNNING;
	}
	Process leagueIngameProcess = ProcessWatcher::GetProcess(RiotIngameAPI::DEFAULT_LEAGUE_INGAME_CLIENT_EXE_NAME);
	return leagueIngameProcess.isValid() ? ClientWorkflowStatus::AWAITING_POSTGAME : ClientWorkflowStatus::API_RUNNING;
}

std::ostream& operator<<(std::ostream& out, const ChampSelectSummoner& summoner) {
	std::string summonerNameUnencoded;
	auto summonerName = summoner.getName();
	std::transform(summonerName.cbegin(), summonerName.cend(), std::back_inserter(summonerNameUnencoded), [](wchar_t c) { return (char)c; });
	out << "[Name: " << summonerNameUnencoded.c_str() << ", PUUID: " << summoner.getPuuid().c_str() << ", SummonerId: " << summoner.getSummonerId();
	return out;
}
std::wostream& operator<<(std::wostream& out, const ChampSelectSummoner& summoner) {
	out << "[Name: " << summoner.getName().c_str() << ", PUUID: " << summoner.getPuuid().c_str() << ", SummonerId: " << summoner.getSummonerId();
	return out;
}

const char *RiotChampSelectAPI::GET_SUMMONER_BY_SLOT_URI = "lol-champ-select/v1/summoners/%i";

