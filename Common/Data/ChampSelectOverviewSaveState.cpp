#include "ChampSelectOverviewSaveState.h"
#include "../../ChampSelect/API/RiotChampSelectAPI.h"
#include "SummonerSpell.h"
#include "../../Common/Logging/ErrorCodeTranslator.h"
#include "../../Common/PerformanceClock.h"

const std::wstring SummonerSaveStateCollector::DEFAULT_FILE_PATH = (ProjectFilePathHandler::GetDefaultFilePathUnicode() + L"cs_savestate.wps");

#pragma warning(disable:4996)
SummonerSaveState::SummonerSaveState(ChampSelectSummoner* summoner) {
	this->championId = summoner->getChampionId();
	auto& summonerSpells = summoner->getSummonerSpells();
	this->position = static_cast<SummonerLanePosition>(summoner->getSlotId() % static_cast<uint8_t>(SummonerLanePosition::POSITION_AMOUNT));
	this->firstSummonerSpellId = summonerSpells.size() > 0 && summonerSpells.at(0) != nullptr ? summonerSpells.at(0)->getId() : 0;
	this->secondSummonerSpellId = summonerSpells.size() > 1 && summonerSpells.at(1) != nullptr ? summonerSpells.at(1)->getId() : 0;
	summonerName = summoner->getName();
	teamType = summoner->getTeamType();
}


bool SummonerSaveStateCollector::loadStates() {
	logger.logInfo("Trying to load Summoner Save State from Path: ", DEFAULT_FILE_PATH.c_str());
	FILE *saveState = _wfopen(DEFAULT_FILE_PATH.c_str(), L"rb+");
	if (!saveState) {
		logger.logError("Error opening file. Reason: ", ErrorCodeTranslator::GetErrorCodeString().c_str());
		return false;
	}
	fread(&gameId, 1, sizeof(uint64_t), saveState);
	uint64_t saveStateTime = 0;
	fread(&saveStateTime, 1, sizeof(uint64_t), saveState);
	uint32_t saveamount = 0;
	fread(&saveamount, sizeof(uint32_t), 1, saveState);
	logger.logInfo("Reading overview data from GameId ", gameId, " with a total of ", saveamount, " entries.");
	for (uint32_t i = 0; i < saveamount && i < 10; i++) {
		loadNextSaveState(saveState);
	}
	fclose(saveState);
	return true;
}

void SummonerSaveStateCollector::loadNextSaveState(FILE* saveFile) {
	wchar_t nameBuffer[0x200] = { 0x00 };
	uint32_t length = 0;

	fread(&length, sizeof(wchar_t), 1, saveFile);
	fread(nameBuffer, sizeof(wchar_t), (std::min)(length, 0x200u), saveFile);
	uint8_t teamType = 0, lanePosition = 0;
	fread(&teamType, sizeof(uint8_t), 1, saveFile);
	fread(&lanePosition, sizeof(uint8_t), 1, saveFile);
	uint32_t championId, firstSummoner, secondSummoner = 0;
	fread(&championId, sizeof(uint32_t), 1, saveFile);
	fread(&firstSummoner, sizeof(uint32_t), 1, saveFile);
	fread(&secondSummoner, sizeof(uint32_t), 1, saveFile);
	SummonerSaveState readState(std::wstring(nameBuffer), static_cast<TeamType>(teamType), static_cast<SummonerLanePosition>(lanePosition), championId, firstSummoner, secondSummoner);
	logger.logDebug("Read entry is: ", readState);
	this->collectedSaveStates.push_back(std::move(readState));
}

bool SummonerSaveStateCollector::saveStates() const {
	FILE* saveState = _wfopen(DEFAULT_FILE_PATH.c_str(), L"wb+");
	if (!saveState) {
		logger.logError("Error opening save-file. Reason: ", ErrorCodeTranslator::GetErrorCodeString().c_str());
		return false;
	}
	uint64_t currentTimestamp = PerformanceClock::GetCurrentMillisecondTimestamp();
	fwrite(&gameId, 1, sizeof(uint64_t), saveState);
	fwrite(&currentTimestamp, 1, sizeof(uint64_t), saveState);
	uint32_t saveAmount = (uint32_t)this->collectedSaveStates.size();
	fwrite(&saveAmount, sizeof(uint32_t), 1, saveState);
	logger.logDebug("Writing GameId ", gameId, " with a total of ", collectedSaveStates.size(), " entries.");
	for (uint32_t i = 0; i < saveAmount; i++) {
		auto& currentState = this->collectedSaveStates.at(i);
		saveNextSaveState(saveState, currentState);
	}
	fclose(saveState);
	return true;
}

void SummonerSaveStateCollector::saveNextSaveState(FILE* fh, const SummonerSaveState& summonerSave) const {
	fputwc(static_cast<uint16_t>(summonerSave.getSummonerName().length()), fh);
	fwrite(summonerSave.getSummonerName().c_str(), sizeof(wchar_t), summonerSave.getSummonerName().length(), fh);
	uint8_t teamType = static_cast<uint8_t>(summonerSave.getTeamType());
	fwrite(&teamType, sizeof(uint8_t), 1, fh);

	uint8_t laneType = static_cast<uint8_t>(summonerSave.getLanePosition());
	fwrite(&laneType, sizeof(uint8_t), 1, fh);

	uint32_t champId = summonerSave.getChampionId();
	fwrite(&champId, sizeof(uint32_t), 1, fh);

	uint32_t firstSummonerSpellId = summonerSave.getFirstSummonerSpellId();
	uint32_t secondSummonerSpellId = summonerSave.getSecondSummonerSpellId();

	fwrite(&firstSummonerSpellId, sizeof(uint32_t), 1, fh);
	fwrite(&secondSummonerSpellId, sizeof(uint32_t), 1, fh);
}


std::wostream& operator<<(std::wostream& out, const SummonerSaveState& saveState) {
	out << "[Summoner: " << saveState.getSummonerName().c_str() << " (Lane: " << saveState.getLanePosition() << "), ChampionId: " << saveState.getChampionId();
	out << " SummonerSpells(" << saveState.getFirstSummonerSpellId() << ", " << saveState.getSecondSummonerSpellId() << ")]";
	return out;
}

std::ostream& operator<<(std::ostream& out, const SummonerSaveState& saveState) {
	std::string summonerName;
	auto summonerNameUnicode = saveState.getSummonerName();
	std::transform(summonerNameUnicode.begin(), summonerNameUnicode.end(), std::back_inserter(summonerName), [](const wchar_t c) { return (char)c; });
	out << "[Summoner: " << summonerName.c_str() << " (Lane: " << saveState.getLanePosition() << "), ChampionId: " << saveState.getChampionId();
	out << " SummonerSpells(" << saveState.getFirstSummonerSpellId() << ", " << saveState.getSecondSummonerSpellId() << ")]";
	return out;
}