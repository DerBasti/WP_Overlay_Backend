#ifndef __RIOT_CHAMP_SELECT_SNAPSHOT__
#define __RIOT_CHAMP_SELECT_SNAPSHOT__

#include <inttypes.h>
#include <unordered_map>
#include <set>
#include "../../Common/Data/json/document.h"
#include "../../Common/Logging/Logger.h"

enum class ChampSelectEventType : uint8_t {
	PICK,
	BAN,
	SUMMONER_SPELL
};

enum class ChampSelectPhase : uint8_t {
	UNKNOWN,
	BANS_FIRST_ROUND,
	PICKS_FIRST_ROUND,
	BANS_SECOND_ROUND,
	PICKS_SECOND_ROUND,
	SWAPPING,
	SPECTATOR_DELAY,
	WAITING_FOR_FINISH,
	FINISHED
};

class ChampSelectPhaseStringifier {
private:
	ChampSelectPhaseStringifier() {}
public:
	static const char* getPhaseAsString(ChampSelectPhase phase) {
		static const char* phaseNames[] = { "Unknown","Ban Phase 1",
			"Pick Phase 1","Ban Phase 2",
			"Pick Phase 2","Swap Phase",
			"Spectator Delay", "Game Startup", "Finished"
		};
		return phaseNames[(int)phase];
	}
	static const wchar_t* getPhaseAsUnicodeString(ChampSelectPhase phase) {
		static const wchar_t* phaseNames[] = { L"Unknown",L"Ban Phase 1",
			L"Pick Phase 1",L"Ban Phase 2",
			L"Pick Phase 2",L"Swap Phase",
			L"Spectator Delay", L"Game Startup", L"Finished"
		};
		return phaseNames[(int)phase];
	}
};

enum class ChampSelectSummonerSlot : uint8_t {
	BLUE_TOP,
	BLUE_JUNGLE,
	BLUE_MID,
	BLUE_ADC,
	BLUE_SUPPORT,
	RED_TOP,
	RED_JUNGLE,
	RED_MID,
	RED_ADC,
	RED_SUPPORT,
	MAX_AMOUNT,
	UNKNOWN = static_cast<uint8_t>(-1)
};

std::ostream& operator<<(std::ostream& out, ChampSelectSummonerSlot slot);
std::wostream& operator<<(std::wostream& out, ChampSelectSummonerSlot slot);
std::ostream& operator<<(std::ostream& out, ChampSelectPhase phase);
std::wostream& operator<<(std::wostream& out, ChampSelectPhase phase);

class SummonerSpell;

class ChampSelectEvent {
private:
	ChampSelectEventType type;
	const char* eventName;
	ChampSelectSummonerSlot summonerSlotId;
	bool finished;
	bool inProgress;
protected:
	__inline void setEventName(const char* eventName) {
		this->eventName = eventName;
	}
public:
	static constexpr const char* EVENT_NAME = "unknown";
	ChampSelectEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId);
	virtual ~ChampSelectEvent();

	virtual bool isEventIdentical(const ChampSelectEvent* other) {
		return other != nullptr && other->getEventType() == getEventType() &&
			other->isFinished() == this->isFinished() && other->isCurrentlyInProgress() == this->isCurrentlyInProgress();
	}

	__inline ChampSelectEventType getEventType() const {
		return type;
	}
	__inline const char* getEventName() const {
		return eventName;
	}
	__inline bool isFinished() const {
		return finished;
	}
	__inline void setFinished(bool flag) {
		this->finished = flag;
	}
	__inline bool isCurrentlyInProgress() const {
		return inProgress;
	}
	__inline void setCurrentlyInProgress(bool flag) {
		inProgress = flag;
	}
	__inline ChampSelectSummonerSlot getActingSummonerSlotId() const {
		return summonerSlotId;
	}
	virtual std::string toString() const {
		char buffer[0x150] = { 0x00 };
		sprintf_s(buffer, "[%s | AffectedSummoner: %i, (InProgress: %s, Finished: %s)]", getEventName(), getActingSummonerSlotId(), (isCurrentlyInProgress() ? "true" : "false"),
			(isFinished() ? "true" : "false"));
		return std::string(buffer);
	}
};

class ChampSelectBanEvent : public ChampSelectEvent {
private:
	uint32_t champId;
public:
	static constexpr const char* EVENT_NAME = "ban";
	ChampSelectBanEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId);
	virtual ~ChampSelectBanEvent();

	__inline uint32_t getBannedChampId() const {
		return champId;
	}

	virtual bool isEventIdentical(const ChampSelectEvent* other) {
		return ChampSelectEvent::isEventIdentical(other) && dynamic_cast<const ChampSelectBanEvent*>(other) != nullptr &&
			dynamic_cast<const ChampSelectBanEvent*>(other)->getBannedChampId() == getBannedChampId();
	}
	virtual std::string toString() const {
		char buffer[0x200] = { 0x00 };
		sprintf_s(buffer, "%s[BannedChamp: %i]", ChampSelectEvent::toString().c_str(), getBannedChampId());
		return std::string(buffer);
	}
};

class ChampSelectPickEvent : public ChampSelectEvent {
private:
	uint32_t champId;
public:
	static constexpr const char* EVENT_NAME = "pick";
	ChampSelectPickEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId);
	virtual ~ChampSelectPickEvent();

	__inline uint32_t getPickedChampId() const {
		return champId;
	}
	virtual bool isEventIdentical(const ChampSelectEvent* other) {
		return ChampSelectEvent::isEventIdentical(other) && dynamic_cast<const ChampSelectPickEvent*>(other) != nullptr &&
			dynamic_cast<const ChampSelectPickEvent*>(other)->getPickedChampId() == getPickedChampId();
	}
	virtual std::string toString() const {
		char buffer[0x200] = { 0x00 };
		sprintf_s(buffer, "%s[PickedChamp: %i]", ChampSelectEvent::toString().c_str(), getPickedChampId());
		return std::string(buffer);
	}
};

class ChampSelectChampionSwapEvent : public ChampSelectEvent {
private:
	uint32_t champId;
public:
	static constexpr const char* EVENT_NAME = "swap";
	ChampSelectChampionSwapEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, int32_t champId);
	virtual ~ChampSelectChampionSwapEvent();

	__inline uint32_t getSwappedChampId() const {
		return champId;
	}
	virtual bool isEventIdentical(const ChampSelectEvent* other) {
		return ChampSelectEvent::isEventIdentical(other) && dynamic_cast<const ChampSelectChampionSwapEvent*>(other) != nullptr &&
			dynamic_cast<const ChampSelectChampionSwapEvent*>(other)->getSwappedChampId() == getSwappedChampId();
	}
	virtual std::string toString() const {
		char buffer[0x200] = { 0x00 };
		sprintf_s(buffer, "%s[SwappedChamp: %i]", ChampSelectEvent::toString().c_str(), getSwappedChampId());
		return std::string(buffer);
	}
};

class ChampSelectSummonerSpellPickedEvent : public ChampSelectEvent {
private:
	uint32_t summonerSpellId;
public:
	static constexpr const char* EVENT_NAME = "summoner_spell_picked";
	ChampSelectSummonerSpellPickedEvent(ChampSelectEventType type, ChampSelectSummonerSlot summonerSlotId, uint32_t spellId) : ChampSelectEvent(type, summonerSlotId) {
		summonerSpellId = spellId;
	}
	virtual ~ChampSelectSummonerSpellPickedEvent() {

	}
	virtual bool isEventIdentical(const ChampSelectEvent* other) {
		return ChampSelectEvent::isEventIdentical(other) && dynamic_cast<const ChampSelectSummonerSpellPickedEvent*>(other) != nullptr &&
			dynamic_cast<const ChampSelectSummonerSpellPickedEvent*>(other)->getSummonerSpellId() == getSummonerSpellId();
	}
	__inline uint32_t getSummonerSpellId() const {
		return summonerSpellId;
	}
	virtual std::string toString() const {
		char buffer[0x200] = { 0x00 };
		sprintf_s(buffer, "%s[PickedSummonerSpell: %i]", ChampSelectEvent::toString().c_str(), getSummonerSpellId());
		return std::string(buffer);
	}
};

class RiotChampSelectSnapshotTimer {
private:
	uint64_t timestampOfReading;
	uint64_t riotTimestamp;
	uint64_t adjustedTime;
	uint64_t timeLeftRegular;
	std::string riotTimerPhaseName;

	__inline uint64_t getRiotTimestamp() const {
		return riotTimestamp;
	}
	__inline uint64_t getRiotAdjustedTime() const {
		return adjustedTime;
	}
	__inline uint64_t getRiotTimeLeftRegular() const {
		return timeLeftRegular;
	}

public:
	RiotChampSelectSnapshotTimer(rapidjson::Document& doc);
	~RiotChampSelectSnapshotTimer();

	__inline std::string getRiotTimerPhaseName() const {
		return riotTimerPhaseName;
	}
	__inline bool isTotalTimeForSpectatorDelay() const {
		return getRiotTimeLeftRegular() >= 150000;
	}
	__inline int64_t getDefaultRemainingTimeInSeconds() const {
		return static_cast<int64_t>(getDefaultRemainingTimeInMillis() / 1000.0f);
	}
	inline int64_t getDefaultRemainingTimeInMillis() const {
		return (riotTimestamp + getRiotTimeLeftRegular() - (getRiotTimeLeftRegular() - getRiotAdjustedTime()) - timestampOfReading + 3000);
	}
	inline int64_t getRemainingTimeInSeconds(RiotChampSelectSnapshotTimer* other) const {
		return getRemainingTimeInMillis(other) / 1000;
	}
	int64_t getRemainingTimeInMillis(RiotChampSelectSnapshotTimer* other) const;
};

class RiotChampSelectSnapshot {
private:
	uint64_t matchId;
	std::wstring phaseName;
	std::unordered_map<uint32_t, ChampSelectEvent*> events;
	std::unordered_map<ChampSelectSummonerSlot, uint32_t> champSwaps;
	std::unordered_map<ChampSelectSummonerSlot, std::vector<uint64_t>> summonerSpells;
	std::unordered_map<ChampSelectSummonerSlot, uint32_t> championsInitiallyPicked;
	std::unordered_map<ChampSelectSummonerSlot, uint32_t> championCurrentlyPicked;
	uint32_t lastKnownEventId;
	std::shared_ptr<RiotChampSelectSnapshotTimer> timerData;
	constexpr static const wchar_t* PICK_BAN_PHASE_NAME = L"BAN_PICK";
	constexpr static const wchar_t* FINALIZATION_PHASE_NAME = L"FINALIZATION";
	constexpr static const wchar_t* GAME_STARTING_PHASE_NAME = L"GAME_STARTING";

	void readNonActionableChangesFromJson(rapidjson::Document& doc);
	void readSummonerSpellChangesFromJson(rapidjson::Document& doc, const char* jsonTeamName);
	void readSummonerActionsFromJson(rapidjson::Document& doc);
	void readChampionSwapsFromJson(rapidjson::Document& doc, const char* jsonTeamName);
	void readTimerDataFromJson(rapidjson::Document& doc);
public:
	RiotChampSelectSnapshot(rapidjson::Document& doc);
	~RiotChampSelectSnapshot();

	__inline uint32_t getInitiallyPickedChampForSummonerSlot(ChampSelectSummonerSlot slot) const {
		return championsInitiallyPicked.find(slot) != championsInitiallyPicked.cend() ? championsInitiallyPicked.at(slot) : 0;
	}

	__inline const std::unordered_map<uint32_t, ChampSelectEvent*>& getEvents() const {
		return events;
	}
	__inline const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& getChampSwaps() const {
		return champSwaps;
	}
	__inline std::shared_ptr<RiotChampSelectSnapshotTimer> getTimer() const {
		return timerData;
	}
	__inline const std::unordered_map<ChampSelectSummonerSlot, std::vector<uint64_t>>& getSelectedSummonerSpells() const {
		return summonerSpells;
	}
	inline const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& getCurrentlySelectedChamps() const {
		return championCurrentlyPicked;
	}
	__inline uint32_t getLastKnownEventId() const {
		return lastKnownEventId;
	}
	__inline bool isPickBanPhase() const {
		return _wcsicmp(PICK_BAN_PHASE_NAME, phaseName.c_str()) == 0;
	}
	__inline bool isChampSwapPhase() const {
		return _wcsicmp(FINALIZATION_PHASE_NAME, phaseName.c_str()) == 0 && !timerData->isTotalTimeForSpectatorDelay();
	}
	__inline bool isSpectatorDelayPhase() const {
		return _wcsicmp(FINALIZATION_PHASE_NAME, phaseName.c_str()) == 0 && timerData->isTotalTimeForSpectatorDelay();
	}
	__inline bool isGameStartingPhase() const {
		return _wcsicmp(GAME_STARTING_PHASE_NAME, phaseName.c_str()) == 0;
	}
	
};

class ChampSelectTeam;

class RiotChampSelectSnapshotDelta {
private:
	ROSEThreadedLogger logger;
	std::shared_ptr<RiotChampSelectSnapshot> previous;
	std::shared_ptr<RiotChampSelectSnapshot> current;
	std::unordered_map<ChampSelectSummonerSlot, uint32_t> championSwaps;
	std::unordered_map<uint32_t, ChampSelectEvent*> eventDelta;
	std::unordered_map<ChampSelectSummonerSlot, std::vector<SummonerSpell*>> summonerSpells;
	std::set<uint32_t> eventDeltaIds;
	ChampSelectEvent* previouslyUnfinishedNowFinishedEvent;
	int64_t timeLeftInMillis;

	ChampSelectPhase estimatedPhase;

	std::string ConvertChampSelectPhaseToString(ChampSelectPhase phase);
	bool addLastKnownEventIfDifferent(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current);
	ChampSelectPhase determineChampSelectPhase(std::shared_ptr<RiotChampSelectSnapshot>& snapshot, ChampSelectTeam* blue, ChampSelectTeam* red);
	void createEventDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current);
	void createSwapDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current);
	void createSummonerSpellDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current);
	void handleCurrentSummonerSwap(ChampSelectSummonerSlot currentSlot, const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& swapFromPrevious, const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& swapFromCurrent);
public:
	RiotChampSelectSnapshotDelta(std::shared_ptr<RiotChampSelectSnapshot> previous, std::shared_ptr<RiotChampSelectSnapshot> current, ChampSelectTeam* blue, ChampSelectTeam* red);
	virtual ~RiotChampSelectSnapshotDelta();

	__inline const std::unordered_map<uint32_t, ChampSelectEvent*>& getEventDelta() const {
		return eventDelta;
	}
	__inline const std::unordered_map<ChampSelectSummonerSlot, uint32_t>& getChampionSwaps() const {
		return championSwaps;
	}
	__inline const std::unordered_map<ChampSelectSummonerSlot, std::vector<SummonerSpell*>> getSelectedSummonerSpells() const {
		return summonerSpells;
	}
	__inline std::set<uint32_t> getEventIds() const {
		return eventDeltaIds;
	}
	__inline int64_t getTimeLeftInSeconds() const {
		return getTimeLeftInMillis() / 1000;
	}
	inline int64_t getTimeLeftInMillis() const {
		return timeLeftInMillis;
	}
	__inline ChampSelectPhase getCurrentChampSelectPhase() const {
		return estimatedPhase;
	}
};


#endif //__RIOT_CHAMP_SELECT_SNAPSHOT__