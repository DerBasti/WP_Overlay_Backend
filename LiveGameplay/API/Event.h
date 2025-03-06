#ifndef __LEAGUE_EVENT_INTERFACE__
#define __LEAGUE_EVENT_INTERFACE__

#include <string>
#include <chrono>
#include <inttypes.h>
#include <unordered_map>

#include "../../Common/Logging/Logger.h"
#include "../../Common/Data/Team.h"
#include "../../Common/Riot/DragonTypes.h"
#include "../../Common/Riot/EpicMonsterTypes.h"

class RiotIngameEvent {
private:
	char* eventName;
	float gameTime;
	uint64_t hash;
public:
	static const char* GAME_STARTED_EVENT_NAME;
	static const char* TURRET_KILLED_EVENT_NAME;
	static const char* INHIBITOR_KILLED_EVENT_NAME;
	static const char* INHIBITOR_RESPAWNED_EVENT_NAME;
	static const char* BARON_KILLED_EVENT_NAME;
	static const char* DRAKE_KILLED_EVENT_NAME;
	static const char* RIFT_HERALD_KILLED_EVENT_NAME;

	RiotIngameEvent(const char* eventName, float ingameTime);
	virtual ~RiotIngameEvent();

	__inline const char* getEventName() const {
		return eventName;
	}
	__inline float getOccuranceTime() const {
		return gameTime;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		return event != nullptr && _stricmp(eventName, event->getEventName()) == 0;
	}
};

enum class InhibitorPosition : uint8_t {
	BOTLANE,
	MIDLANE,
	TOPLANE,
	INHIBITOR_POSITIONS_AMOUNT,
	UNKNOWN = 0xFF,
};

class Inhibitor {
private:
	uint32_t id;
	TeamType team;
	InhibitorPosition position;
	std::string name;
public:
	Inhibitor(uint32_t id, TeamType team, InhibitorPosition position, const char* name) {
		this->id = id;
		this->team = team;
		this->position = position;
		this->name = name;
	}
	virtual ~Inhibitor() {

	}
	__inline uint32_t getId() const {
		return id;
	}
	__inline TeamType getTeam() const {
		return team;
	}
	__inline InhibitorPosition getLanePosition() const {
		return position;
	}
	__inline std::string getName() const {
		return name;
	}
};

class InhibitorKilledEvent : public RiotIngameEvent {
private:
	Inhibitor targetInhibitor;
	float respawnTime;
	bool activeFlag;
	float remainingTime;
	constexpr static float DEFAULT_RESPAWN_TIME_IN_SECONDS = 300.0f;
public:
	InhibitorKilledEvent(const char* eventName, float ingameTime, Inhibitor inhibitor) : RiotIngameEvent(eventName, ingameTime), targetInhibitor(inhibitor) {
		respawnTime = ingameTime + DEFAULT_RESPAWN_TIME_IN_SECONDS;
		updateRemainingTime(ingameTime);
	}
	virtual ~InhibitorKilledEvent() {

	}
	__inline const Inhibitor& getInhibitor() const {
		return targetInhibitor;
	}
	__inline float getRemainingTime() const {
		return remainingTime;
	}
	__inline void updateRemainingTime(float currentIngameTime) {
		remainingTime = DEFAULT_RESPAWN_TIME_IN_SECONDS - (currentIngameTime - getOccuranceTime());
		activeFlag = remainingTime > 0.0f && remainingTime < DEFAULT_RESPAWN_TIME_IN_SECONDS;
	}
	__inline float getRespawnTime() const {
		return respawnTime;
	}
	__inline bool isActive() const {
		return activeFlag;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		return RiotIngameEvent::isIdenticalTo(event) && dynamic_cast<const InhibitorKilledEvent*>(event) != nullptr &&
			_stricmp(dynamic_cast<const InhibitorKilledEvent*>(event)->getInhibitor().getName().c_str(), getInhibitor().getName().c_str()) == 0 && (uint32_t)abs(getOccuranceTime() - event->getOccuranceTime()) <= 10.0f;
	}
};

class InhibitorRespawnedEvent : public RiotIngameEvent {
private:
	Inhibitor targetInhibitor;
public:
	InhibitorRespawnedEvent(const char* eventName, float ingameTime, Inhibitor inhibitor) : RiotIngameEvent(eventName, ingameTime), targetInhibitor(inhibitor) {

	}
	virtual ~InhibitorRespawnedEvent() {

	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		return RiotIngameEvent::isIdenticalTo(event) && dynamic_cast<const InhibitorRespawnedEvent*>(event) != nullptr &&
			_stricmp(dynamic_cast<const InhibitorRespawnedEvent*>(event)->getInhibitor().getName().c_str(), getInhibitor().getName().c_str()) == 0 && (uint32_t)abs(getOccuranceTime() - event->getOccuranceTime()) <= 10.0f;
	}
	__inline const Inhibitor& getInhibitor() const {
		return targetInhibitor;
	}
};

class TurretKilledEvent : public RiotIngameEvent {
private:
	TeamType team;
	char* turretName;
public:
	TurretKilledEvent(const char* eventName, float ingameTime, const char* turretName, TeamType destroyerTeam) : RiotIngameEvent(eventName, ingameTime) {
		this->team = destroyerTeam;
		size_t len = strlen(turretName);
		this->turretName = new char[len + 1];
		strncpy_s(this->turretName, len + 1, turretName, len);
		this->turretName[len] = 0x00;
	}
	virtual ~TurretKilledEvent() {
		delete[] turretName;
		turretName = nullptr;
	}
	__inline TeamType getTurretDestroyerTeam() const {
		return team;
	}
	__inline const char* getTurretName() const {
		return turretName;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		const TurretKilledEvent* other = dynamic_cast<const TurretKilledEvent*>(event);
		return RiotIngameEvent::isIdenticalTo(event) && other != nullptr && _stricmp(getTurretName(), other->getTurretName()) == 0;
	}
};

class RiftHeraldKilledEvent : public RiotIngameEvent {
private:
	TeamType killTeam;
public:
	RiftHeraldKilledEvent(const char* eventName, float ingameTime, TeamType buffedTeam) : RiotIngameEvent(eventName, ingameTime) {
		killTeam = buffedTeam;
	}
	inline TeamType getKillTeam() const {
		return killTeam;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		const RiftHeraldKilledEvent* other = dynamic_cast<const RiftHeraldKilledEvent*>(event);
		return RiotIngameEvent::isIdenticalTo(event) && other != nullptr && getKillTeam() == other->getKillTeam() && (uint32_t)abs(getOccuranceTime() - event->getOccuranceTime()) <= 10.0f;
	}
};

class DragonKilledEvent : public RiotIngameEvent {
private:
	DragonType killedDragonType;
	TeamType buffedTeam;
	static std::unordered_map<std::string, DragonType> DragonTypesByString;
	static std::unordered_map<DragonType, std::string> DragonTypes;
	const static constexpr float MINIMUM_DIFFERENCE = 300.0f;
public:
	DragonKilledEvent(const char* eventName, float ingameTime, DragonType killedDragonType, TeamType buffedTeam) : RiotIngameEvent(eventName, ingameTime) {
		this->killedDragonType = killedDragonType;
		this->buffedTeam = buffedTeam;
	}
	virtual ~DragonKilledEvent() {

	}
	static DragonType ExtractTypeFromString(const char* name);

	inline DragonType getKilledDragonType() const {
		return killedDragonType;
	}
	inline std::string getKilledDragonName() const {
		return DragonTypes.at(killedDragonType);
	}
	inline TeamType getBuffedTeam() const {
		return buffedTeam;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		const DragonKilledEvent* other = dynamic_cast<const DragonKilledEvent*>(event);
		return RiotIngameEvent::isIdenticalTo(event) && other != nullptr && ((other->getKilledDragonType() == getKilledDragonType() && (uint32_t)abs(getOccuranceTime() - event->getOccuranceTime()) <= MINIMUM_DIFFERENCE));
	}
};

class BaronKilledEvent : public RiotIngameEvent {
private:
	TeamType buffedTeam;
	float buffExpiresAtIngameTime;
	static constexpr float DEFAULT_BARON_TIME_IN_SECONDS = 180.0f;
public:
	BaronKilledEvent(const char* eventName, float ingameTime, TeamType buffedTeam) : RiotIngameEvent(eventName, ingameTime) {
		this->buffedTeam = buffedTeam;
		buffExpiresAtIngameTime = ingameTime + DEFAULT_BARON_TIME_IN_SECONDS;
	}
	virtual ~BaronKilledEvent() {

	}
	inline TeamType getBuffedTeam() const {
		return buffedTeam;
	}
	inline float getBuffExpirationTime() const {
		return buffExpiresAtIngameTime;
	}
	virtual bool isIdenticalTo(const RiotIngameEvent* event) const {
		const BaronKilledEvent* other = dynamic_cast<const BaronKilledEvent*>(event);
		return RiotIngameEvent::isIdenticalTo(event) && other != nullptr && other->getBuffedTeam() == getBuffedTeam() && (uint32_t)abs(getOccuranceTime() - event->getOccuranceTime()) <= 100;
	}
};

#endif //__LEAGUE_EVENT_INTERFACE__