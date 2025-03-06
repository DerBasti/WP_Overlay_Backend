#include "SummonerSpell.h"
#include "VersionDatabase.h"
#include "CURLWrapper.h"

std::unordered_map<std::wstring, SummonerSpell*> SummonerSpell::SummonerSpellsByName;
std::unordered_map<uint64_t, SummonerSpell*> SummonerSpell::SummonerSpellsById;

void SummonerSpell::Init() {
	ROSEThreadedLogger logger;
	logger.logDebug("Initializing SummonerSpells.");
	if (!SummonerSpellsById.empty()) {
		logger.logDebug("SummonerSpells were already initialized. Assuming refresh request; destroying currently stored SummonerSpell infos.");
		OnDestroy();
	}
	auto summonerSpellsJson = GetData();
	if (summonerSpellsJson.IsNull() || summonerSpellsJson.HasParseError()) {
		logger.logWarn("SummonerSpell Json is either empty or has parsing errors. No summoner spells will be loaded.");
		return;
	}
	auto& summonerSpells = summonerSpellsJson["data"];
	for (auto it = summonerSpells.MemberBegin(); it != summonerSpells.MemberEnd(); it++) {
		auto& currentEntry = it->value;

		auto summonerIcon = std::string(currentEntry.FindMember("id")->value.GetString()) + std::string(".png");
		std::wstring summonerIconUnicode = std::wstring(summonerIcon.c_str(), summonerIcon.c_str() + summonerIcon.length());

		std::string name = currentEntry.FindMember("name")->value.GetString();
		std::wstring summonerSpellNameUnicode = std::wstring(name.c_str(), name.c_str() + name.length());

		uint32_t id = atoi(currentEntry.FindMember("key")->value.GetString());

		std::wstring basicPath = (ProjectFilePathHandler::GetDefaultFilePathUnicode() + L"Icons/");
		SummonerSpell* newSummoner = new SummonerSpell(id, summonerSpellNameUnicode.c_str(), (basicPath + summonerIconUnicode).c_str());
		SummonerSpellsById.insert(std::make_pair(id, newSummoner));
		SummonerSpellsByName.insert(std::make_pair(summonerSpellNameUnicode, newSummoner));
	}
	SummonerSpell* emptyResult = new SummonerSpell(0, L"", L"");
	SummonerSpellsById.insert(std::make_pair(0, emptyResult));
	SummonerSpellsByName.insert(std::make_pair(std::wstring(L""), emptyResult));
	logger.logInfo("A total of ", SummonerSpellsById.size(), " SummonerSpells were loaded.");
}

std::string SummonerSpell::GetLatestJsonUrl() {
	char buffer[0x500] = { 0x00 };
	sprintf_s(buffer, SUMMONER_JSON_URL, VersionDatabase::getInstance()->getLatestVersion().c_str());
	return std::string(buffer);
}

rapidjson::Document SummonerSpell::GetData() {
	auto championUrl = GetLatestJsonUrl();
	ROSEThreadedLogger logger;
	logger.logDebug("Loading summoner spells from url: ", championUrl.c_str());
	CURLBufferedWrapper wrapper(championUrl.c_str());
	wrapper.fireRequest();
	rapidjson::Document document;
	if (wrapper.getReadDataLength() > 0) {
		document.Parse(wrapper.getReadData().c_str());
	}
	return document;
}