#include "Event.h"

const char* RiotIngameEvent::GAME_STARTED_EVENT_NAME = "GameStart";
const char* RiotIngameEvent::BARON_KILLED_EVENT_NAME = "OnKillWorm_Spectator";
const char* RiotIngameEvent::DRAKE_KILLED_EVENT_NAME = "OnKillDragon_Spectator";
const char* RiotIngameEvent::RIFT_HERALD_KILLED_EVENT_NAME = "OnKillRiftHerald_Spectator";
const char* RiotIngameEvent::TURRET_KILLED_EVENT_NAME = "TurretKilled";
const char* RiotIngameEvent::INHIBITOR_KILLED_EVENT_NAME = "InhibKilled";
const char* RiotIngameEvent::INHIBITOR_RESPAWNED_EVENT_NAME = "SELFCREATED_EVENT_InhibKilled";

RiotIngameEvent::RiotIngameEvent(const char* eventName, float ingameTime) {
	size_t eventNameLen = strlen(eventName);
	this->eventName = new char[eventNameLen + 1];
	strncpy_s(this->eventName, eventNameLen + 1, eventName, eventNameLen);

	this->gameTime = ingameTime;
}

RiotIngameEvent::~RiotIngameEvent() {
	delete[] eventName;
	eventName = nullptr;

	gameTime = -1.0f;
}

DragonType DragonKilledEvent::ExtractTypeFromString(const char* name) {
	return DragonTypesByString.find(std::string(name)) != DragonTypesByString.cend() ? DragonTypesByString.at(name) : DragonType::UNKNOWN;
}

std::unordered_map<std::string, DragonType> DragonKilledEvent::DragonTypesByString{
	{"", DragonType::UNKNOWN},
	{"SRU_Dragon_Hextech", DragonType::HEXTECH},
	{"SRU_Dragon_Chemtech", DragonType::CHEMTECH},
	{"SRU_Dragon_Fire", DragonType::INFERNAL},
	{"SRU_Dragon_Earth", DragonType::MOUNTAIN},
	{"SRU_Dragon_Air", DragonType::CLOUD},
	{"SRU_Dragon_Water", DragonType::OCEAN},
	{"SRU_Dragon_Elder", DragonType::ELDER}
};

std::unordered_map<DragonType, std::string> DragonKilledEvent::DragonTypes{
	{DragonType::UNKNOWN, ""},
	{DragonType::HEXTECH, "SRU_Dragon_Hextech"},
	{DragonType::CHEMTECH, "SRU_Dragon_Chemtech"},
	{DragonType::INFERNAL, "SRU_Dragon_Fire"},
	{DragonType::MOUNTAIN, "SRU_Dragon_Earth"},
	{DragonType::CLOUD, "SRU_Dragon_Air"},
	{DragonType::OCEAN, "SRU_Dragon_Water"},
	{DragonType::ELDER, "SRU_Dragon_Elder"}
};