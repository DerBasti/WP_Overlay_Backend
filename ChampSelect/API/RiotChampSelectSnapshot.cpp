#include "RiotChampSelectSnapshot.h"
#include "../../Common/Data/json/stringbuffer.h"
#include "../../Common/Data/json/prettywriter.h"
#include "../../Common/Data/SummonerSpell.h"
#include <iostream>
#include <chrono>
#include "RiotChampSelectAPI.h"

std::ostream& operator<<(std::ostream& out, ChampSelectSummonerSlot slot) {
	out << (uint32_t)slot;
	return out;
}

std::wostream& operator<<(std::wostream& out, ChampSelectSummonerSlot slot) {
	out << (uint32_t)slot;
	return out;
}

std::ostream& operator<<(std::ostream& out, ChampSelectPhase phase) {
	out << ChampSelectPhaseStringifier::getPhaseAsString(phase);
	return out;
}

std::wostream& operator<<(std::wostream& out, ChampSelectPhase phase) {
	out << ChampSelectPhaseStringifier::getPhaseAsUnicodeString(phase);
	return out;
}

ChampSelectEvent::ChampSelectEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId) {
	this->type = type;
	this->summonerSlotId = summonerSlotId;
	inProgress = false;
	finished = false;
	setEventName(ChampSelectEvent::EVENT_NAME);
}

ChampSelectEvent::~ChampSelectEvent() {
}

ChampSelectBanEvent::ChampSelectBanEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId) : ChampSelectEvent(type, summonerSlotId) {
	this->champId = champId;
	setEventName(ChampSelectBanEvent::EVENT_NAME);
}

ChampSelectBanEvent::~ChampSelectBanEvent() {

}

ChampSelectPickEvent::ChampSelectPickEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId) : ChampSelectEvent(type, summonerSlotId) {
	this->champId = champId;
	setEventName(ChampSelectPickEvent::EVENT_NAME);
}

ChampSelectPickEvent::~ChampSelectPickEvent() {

}

ChampSelectChampionSwapEvent::ChampSelectChampionSwapEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId) : ChampSelectEvent(type, summonerSlotId) {
	this->champId = champId;
	setEventName(ChampSelectChampionSwapEvent::EVENT_NAME);
}

ChampSelectChampionSwapEvent::~ChampSelectChampionSwapEvent() {

}


RiotChampSelectSnapshot::RiotChampSelectSnapshot(rapidjson::Document& doc) {
	matchId = doc["gameId"].GetUint64();
	readSummonerActionsFromJson(doc);
	readTimerDataFromJson(doc);
	readNonActionableChangesFromJson(doc);
}

void RiotChampSelectSnapshot::readNonActionableChangesFromJson(rapidjson::Document& doc) {
	if (this->isChampSwapPhase()) {
		readChampionSwapsFromJson(doc, "myTeam");
		readChampionSwapsFromJson(doc, "theirTeam");

		readSummonerSpellChangesFromJson(doc, "myTeam");
		readSummonerSpellChangesFromJson(doc, "theirTeam");
	}
}

RiotChampSelectSnapshot::~RiotChampSelectSnapshot() {
	for (auto event : events) {
		delete event.second;
	}
	events.clear();
}

void RiotChampSelectSnapshot::readSummonerActionsFromJson(rapidjson::Document& doc) {
	auto actions = doc["actions"].GetArray();

	for (auto actionIt = actions.Begin(); actionIt != actions.End(); actionIt++) {
		auto subActionArray = actionIt->GetArray();
		for (auto subActionIt = subActionArray.Begin(); subActionIt != subActionArray.End(); subActionIt++) {
			auto subAction = subActionIt->GetObj();
			const char* currentEventName = subAction["type"].GetString();
			ChampSelectEvent* event = nullptr; 
			ChampSelectSummonerSlot actorSlotId = ChampSelectSummonerSlot::UNKNOWN;
			if (_stricmp(ChampSelectBanEvent::EVENT_NAME, currentEventName) == 0) {
				actorSlotId = static_cast<ChampSelectSummonerSlot>(subAction["actorCellId"].GetInt());
				event = new ChampSelectBanEvent(ChampSelectEventType::BAN, actorSlotId, subAction["championId"].GetInt());
			}
			else if (_stricmp(ChampSelectPickEvent::EVENT_NAME, currentEventName) == 0) {
				actorSlotId = static_cast<ChampSelectSummonerSlot>(subAction["actorCellId"].GetInt());
				event = new ChampSelectPickEvent(ChampSelectEventType::PICK, actorSlotId, subAction["championId"].GetInt());
				championsInitiallyPicked.insert(std::make_pair(actorSlotId, subAction["championId"].GetInt()));
			}
			if (event != nullptr) {
				event->setFinished(subAction["completed"].GetBool());
				event->setCurrentlyInProgress(subAction["isInProgress"].GetBool());
				uint32_t eventId = subAction["id"].GetInt();
				this->lastKnownEventId = eventId;
				events.insert(std::make_pair(eventId, event));
			}
		}
	}
}

void RiotChampSelectSnapshot::readChampionSwapsFromJson(rapidjson::Document& doc, const char* jsonTeamName) {
	auto currentTeam = doc[jsonTeamName].GetArray();
	if (!currentTeam.Empty()) {
		std::unordered_map<ChampSelectSummonerSlot, uint32_t> currentlySelectedChampions;
		for (auto summonerIt = currentTeam.Begin(); summonerIt != currentTeam.End(); summonerIt++) {
			auto summoner = summonerIt->GetObj();
			ChampSelectSummonerSlot slot = static_cast<ChampSelectSummonerSlot>(summoner["cellId"].GetInt());
			uint32_t champId = summoner["championId"].GetInt();
			currentlySelectedChampions.insert(std::make_pair(slot, champId));
			championCurrentlyPicked.insert_or_assign(slot, champId);
		}
		if (championsInitiallyPicked.empty()) {			
			return;
		}
		for (auto it : currentlySelectedChampions) {
			uint32_t initiallyPickedChamp = championsInitiallyPicked.at(it.first);
			uint32_t currentlyPickedChamp = it.second;
			if (initiallyPickedChamp != currentlyPickedChamp) {
				this->champSwaps.insert(std::make_pair(it.first, currentlyPickedChamp));
				championCurrentlyPicked.insert_or_assign(it.first, currentlyPickedChamp);
			}
		}
	}
	
}

void RiotChampSelectSnapshot::readTimerDataFromJson(rapidjson::Document& doc) {
	uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto timerDataJson = doc["timer"].GetObj();
	std::string phase = std::string(timerDataJson["phase"].GetString());
	this->phaseName = std::wstring(phase.c_str(), phase.c_str() + phase.length());
	timerData = std::shared_ptr<RiotChampSelectSnapshotTimer>(new RiotChampSelectSnapshotTimer(doc));
#ifdef _DEBUG
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
	doc["timer"].Accept(writer);
	auto str = sb.GetString();
	std::cout << "[TimerData]\n" << str << "\n";
#endif
}

void RiotChampSelectSnapshot::readSummonerSpellChangesFromJson(rapidjson::Document& doc, const char* teamName) {
	auto currentTeam = doc[teamName].GetArray();
	if (!currentTeam.Empty()) {
		for (auto summonerIt = currentTeam.Begin(); summonerIt != currentTeam.End(); summonerIt++) {
			auto summoner = summonerIt->GetObj();
			ChampSelectSummonerSlot slot = static_cast<ChampSelectSummonerSlot>(summoner["cellId"].GetInt());
			std::vector<uint64_t> summoners;

			uint64_t firstSummonerSpellId = 0, secondSummonerSpellId = 0;
			if (summoner.HasMember("spell1Id")) {
				firstSummonerSpellId = summoner["spell1Id"].GetUint64() <= 0 ? 0 : summoner["spell1Id"].GetUint64();
				if (firstSummonerSpellId > 0x100) {
					firstSummonerSpellId = 0;
				}
			}
			if (summoner.HasMember("spell2Id")) {
				secondSummonerSpellId = summoner["spell2Id"].GetUint64() <= 0 ? 0 : summoner["spell2Id"].GetUint64();;
				if (secondSummonerSpellId > 0x100) {
					secondSummonerSpellId = 0;
				}
			}
			summoners.push_back(firstSummonerSpellId);
			summoners.push_back(secondSummonerSpellId);

			this->summonerSpells.insert(std::make_pair(slot, std::move(summoners)));
		}
	}
}

RiotChampSelectSnapshotDelta::RiotChampSelectSnapshotDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current, ChampSelectTeam* blue, ChampSelectTeam* red) {
	this->previous = previous;
	this->current = current;
	this->estimatedPhase = ChampSelectPhase::UNKNOWN;
	this->timeLeftInMillis = 1000;
	previouslyUnfinishedNowFinishedEvent = nullptr;
	logger.setLoggerName("RiotChampSelectSnapshotDelta");
	if (current) {
		this->timeLeftInMillis = current->getTimer()->getDefaultRemainingTimeInMillis();
		if (previous) {
			timeLeftInMillis = current->getTimer()->getRemainingTimeInMillis(previous->getTimer().get());
			createEventDelta(previous, current);
			createSwapDelta(previous, current);
		}
		else {
			for (auto it : current->getEvents()) {
				this->eventDelta.insert(std::make_pair(it.first, it.second));
				this->eventDeltaIds.insert(it.first);
			}
		}
		createSummonerSpellDelta(previous, current);
		auto lastPhase = determineChampSelectPhase(previous, blue, red);
		auto currentPhase = determineChampSelectPhase(current, blue, red);
		if (lastPhase == ChampSelectPhase::SPECTATOR_DELAY && currentPhase == ChampSelectPhase::SPECTATOR_DELAY) {
			if (current->getTimer()->getRemainingTimeInSeconds(previous->getTimer().get()) <= 1) {
				logger.logDebug("New Phase detected: ", ConvertChampSelectPhaseToString(currentPhase).c_str(), " (Riot's Phase Name: '", current->getTimer()->getRiotTimerPhaseName().c_str(), " (DefaultTimer: ", current->getTimer()->getDefaultRemainingTimeInMillis(), "'). Phase was previously: '", ConvertChampSelectPhaseToString(lastPhase).c_str(), "'");
				currentPhase = ChampSelectPhase::FINISHED;
			}
		}
		if (lastPhase != currentPhase && currentPhase != ChampSelectPhase::FINISHED) {
			logger.logDebug("New Phase detected: ", ConvertChampSelectPhaseToString(currentPhase).c_str(), " (Riot's Phase Name: '", current->getTimer()->getRiotTimerPhaseName().c_str(), " (DefaultTimer: ", current->getTimer()->getDefaultRemainingTimeInMillis(), "'). Phase was previously: '", ConvertChampSelectPhaseToString(lastPhase).c_str(), "'");
		}
		this->estimatedPhase = currentPhase;
	}
	else {
		logger.logWarn("Current ChampSelectSnapshot is not set! Invalid data will be read.");
	}
}

RiotChampSelectSnapshotDelta::~RiotChampSelectSnapshotDelta() {
	eventDelta.clear();
}

bool RiotChampSelectSnapshotDelta::addLastKnownEventIfDifferent(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current) {
	if (previous->getEvents().empty() || current->getEvents().empty()) {
		return false;
	}
	auto lastEventPrevious = previous->getEvents().at(previous->getLastKnownEventId());
	auto lastEventCurrent = current->getEvents().at(previous->getLastKnownEventId());
	if (!lastEventPrevious->isEventIdentical(lastEventCurrent)) {
		eventDelta.insert(std::make_pair(previous->getLastKnownEventId(), lastEventCurrent));
		return true;
	}
	return false;
}

void RiotChampSelectSnapshotDelta::createEventDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current) {
	for (auto previousEventIterator : previous->getEvents()) {
		auto previousEvent = previousEventIterator.second;
		auto currentEvent = current->getEvents().at(previousEventIterator.first);
		if (!previousEvent->isEventIdentical(currentEvent)) {
			eventDeltaIds.insert(previousEventIterator.first);
			eventDelta.insert(std::make_pair(previousEventIterator.first, currentEvent));
		}
	}
	if (previous->getEvents().size() < current->getEvents().size()) {
		int32_t sizeDiff = static_cast<int32_t>(current->getEvents().size() - previous->getEvents().size());
		if (sizeDiff >= 0) {
			auto lastOverlapId = previous->getLastKnownEventId();
			for (uint32_t i = lastOverlapId + 1; i <= (lastOverlapId + sizeDiff); i++) {
				auto event = current->getEvents().find(i);
				if (event != current->getEvents().cend()) {
					eventDeltaIds.insert(i);
					eventDelta.insert(std::make_pair(i, event->second));
				}
			}
		}
	}
}

void RiotChampSelectSnapshotDelta::createSwapDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current) {
	if (!current->isChampSwapPhase()) {
		return;
	}
	auto previousSwaps = previous->getChampSwaps();
	auto currentSwaps = current->getChampSwaps();
	for (ChampSelectSummonerSlot i = ChampSelectSummonerSlot::BLUE_TOP; i < ChampSelectSummonerSlot::MAX_AMOUNT; i = static_cast<ChampSelectSummonerSlot>((uint8_t)i+1)) {
		handleCurrentSummonerSwap(i, previousSwaps, currentSwaps);
	}
}

void RiotChampSelectSnapshotDelta::createSummonerSpellDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current) {
	if (!previous) {
		for (uint8_t currentSummonerSlot = (uint8_t)ChampSelectSummonerSlot::BLUE_TOP; currentSummonerSlot <= (uint8_t)ChampSelectSummonerSlot::MAX_AMOUNT; currentSummonerSlot += 1) {
			ChampSelectSummonerSlot slot = (ChampSelectSummonerSlot)currentSummonerSlot;
			auto currentIt = current->getSelectedSummonerSpells().find(slot);
			if (currentIt == current->getSelectedSummonerSpells().cend()) {
				continue;
			}
			std::vector<SummonerSpell*> summoners;
			summoners.push_back(SummonerSpell::GetSpellById(currentIt->second.at(0)));
			summoners.push_back(SummonerSpell::GetSpellById(currentIt->second.at(1)));
			this->summonerSpells.insert(std::make_pair(slot, summoners));
		}
		return;
	}
	for (uint8_t currentSummonerSlot = (uint8_t)ChampSelectSummonerSlot::BLUE_TOP; currentSummonerSlot <= (uint8_t)ChampSelectSummonerSlot::MAX_AMOUNT; currentSummonerSlot+=1) {
		ChampSelectSummonerSlot slot = (ChampSelectSummonerSlot)currentSummonerSlot;
		auto prevIt = previous->getSelectedSummonerSpells().find(slot);
		auto currentIt = current->getSelectedSummonerSpells().find(slot);
		std::vector<SummonerSpell*> summoners;
		if (prevIt == previous->getSelectedSummonerSpells().cend() && currentIt != current->getSelectedSummonerSpells().cend()) {
			summoners.push_back(SummonerSpell::GetSpellById(currentIt->second.at(0)));
			summoners.push_back(SummonerSpell::GetSpellById(currentIt->second.at(1)));
			this->summonerSpells.insert(std::make_pair(slot, summoners));
		}
		else if (prevIt != previous->getSelectedSummonerSpells().cend() && currentIt != current->getSelectedSummonerSpells().cend()) {
			SummonerSpell* firstSpellId = nullptr;
			SummonerSpell* secondSpellId = nullptr;
			if (prevIt->second.at(0) != currentIt->second.at(0) || prevIt->second.at(1) != currentIt->second.at(1)) {
				firstSpellId = SummonerSpell::GetSpellById(currentIt->second.at(0));
				secondSpellId = SummonerSpell::GetSpellById(currentIt->second.at(1));
			}
			if (firstSpellId != 0 || secondSpellId != 0) {
				summoners.push_back(firstSpellId);
				summoners.push_back(secondSpellId);
			
				this->summonerSpells.insert(std::make_pair(slot, summoners));
			} 
		}
	}
}

void RiotChampSelectSnapshotDelta::handleCurrentSummonerSwap(ChampSelectSummonerSlot currentSlot, const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& swapFromPrevious, const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& swapFromCurrent) {
	auto previousSwap = swapFromPrevious.find(currentSlot);
	auto currentSwap = swapFromCurrent.find(currentSlot);
	if (previousSwap != swapFromPrevious.cend()) {
		if (currentSwap != swapFromCurrent.cend()) {
			auto previousChampId = previousSwap->second;
			auto currentChampId = currentSwap->second;
			if (previousChampId != currentChampId) {
				logger.logInfo("Detected another swap for Summoner[", currentSlot, "]. New ChampId: ", currentChampId);
				this->championSwaps.insert(std::make_pair(currentSlot, currentChampId));
			}
		}
		else {
			uint32_t initiallyPickedChampId = current->getInitiallyPickedChampForSummonerSlot(currentSlot);
			logger.logWarn("Detected unexpected swap-state for Summoner[", currentSlot, "]. PrevId: ", previousSwap->second, ". Assigning initially picked ChampId: ", initiallyPickedChampId);
			this->championSwaps.insert(std::make_pair(currentSlot, initiallyPickedChampId));
		}
	}
	else if ((previousSwap == swapFromPrevious.cend() && currentSwap != swapFromCurrent.cend())) {
		logger.logInfo("Detected first Swap for Summoner[", currentSlot, "]. New ChampId: ", currentSwap->second);
		this->championSwaps.insert(std::make_pair(currentSlot, currentSwap->second));
	}
}

ChampSelectPhase RiotChampSelectSnapshotDelta::determineChampSelectPhase(std::shared_ptr<RiotChampSelectSnapshot>& current, ChampSelectTeam* blue, ChampSelectTeam* red) {
	ChampSelectPhase phase = ChampSelectPhase::UNKNOWN;
	if (!current) {
		logger.logDebug("Current (previous?) snapshot is invalid. Unknown ChampSelectPhase will be returned for SnapshotDelta.");
		return phase;
	}
	if (current->isPickBanPhase()) {
		phase = ChampSelectPhase::BANS_FIRST_ROUND;
		if (current->getLastKnownEventId() <= 0) {
			return phase;
		}
		uint32_t amountOfPlayer = (blue ? blue->getMemberAmount() : 0) + (red ? red->getMemberAmount() : 0);
		uint32_t firstBanPhaseAmountOfBansRequired = 6;
		uint32_t secondBanPhaseAmountOfBansRequired = 10;
		uint32_t firstPickSessionAmount = 6;
		uint32_t amountOfBans = 0;
		uint32_t amountOfPicks = 0;
		for (auto it : current->getEvents()) {
			auto event = it.second;
			if (event->getEventType() == ChampSelectEventType::BAN && event->isFinished()) {
				amountOfBans++;
			}
			else if (event->getEventType() == ChampSelectEventType::PICK && event->isFinished()) {
				amountOfPicks++;
			}
		}
		//5v5 -> 6 Bans, 6 Picks, 10 Bans, 10 Picks

		//6 Bans, 6 Picks
 		if (amountOfBans < firstBanPhaseAmountOfBansRequired) {
			phase = ChampSelectPhase::BANS_FIRST_ROUND;
		}
		else if (amountOfPicks < firstPickSessionAmount) {
			phase = ChampSelectPhase::PICKS_FIRST_ROUND;
		}
		else if (amountOfPicks >= firstPickSessionAmount && amountOfBans < secondBanPhaseAmountOfBansRequired) {
			phase = ChampSelectPhase::BANS_SECOND_ROUND;
		}
		else if (amountOfPicks < amountOfPlayer) {
			phase = ChampSelectPhase::PICKS_SECOND_ROUND;
		}
	}
	else {
		int64_t remainingTime = current->getTimer()->getDefaultRemainingTimeInSeconds();
		phase = current->isChampSwapPhase() ? ChampSelectPhase::SWAPPING : (remainingTime >= 5 ? ChampSelectPhase::SPECTATOR_DELAY : ChampSelectPhase::WAITING_FOR_FINISH);
	}
	return phase;
}

std::string RiotChampSelectSnapshotDelta::ConvertChampSelectPhaseToString(ChampSelectPhase phase) {
	std::string result = "ChampSelectPhase(";
	switch (phase) {
		case ChampSelectPhase::BANS_FIRST_ROUND:
			result += "Ban";
		break;
		case ChampSelectPhase::FINISHED:
			result += "Finished";
		break;
		case ChampSelectPhase::PICKS_FIRST_ROUND:
			result += "Picks";
		break;
		case ChampSelectPhase::SPECTATOR_DELAY:
			result += "SpectatorDelay";
		break;
		case ChampSelectPhase::SWAPPING:
			result += "Swapping";
		break;
		default:
			result += "Unknown";
	}
	result += ")";
	return result;
}


RiotChampSelectSnapshotTimer::RiotChampSelectSnapshotTimer(rapidjson::Document& doc) {
	timestampOfReading = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto timerData = doc["timer"].GetObj();
	riotTimestamp = timerData["internalNowInEpochMs"].GetInt64();
	timeLeftRegular = timerData["totalTimeInPhase"].GetInt();
	adjustedTime = timerData["adjustedTimeLeftInPhase"].GetInt();
	riotTimerPhaseName = std::string(timerData["phase"].GetString());
}

RiotChampSelectSnapshotTimer::~RiotChampSelectSnapshotTimer() {

}

int64_t RiotChampSelectSnapshotTimer::getRemainingTimeInMillis(RiotChampSelectSnapshotTimer* previousTimer) const {
	int64_t remainingTime = getDefaultRemainingTimeInMillis();
	if (previousTimer != nullptr) {
		bool timestampUpdated = previousTimer->getRiotTimestamp() < getRiotTimestamp();
		bool expectedTimeNotUpdated = previousTimer->getRiotTimeLeftRegular() == getRiotTimeLeftRegular();
		bool adjustedTimeUpdated = previousTimer->getRiotAdjustedTime() > getRiotAdjustedTime();
		if (timestampUpdated && expectedTimeNotUpdated && adjustedTimeUpdated) {
			remainingTime = (getRiotTimestamp() + getRiotAdjustedTime() - timestampOfReading + 3000);
		}
	}
	if (remainingTime <= 0) {
		remainingTime = 0;
	}
	return remainingTime;
}