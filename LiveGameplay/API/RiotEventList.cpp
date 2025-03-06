#include "RiotEventList.h"
#include "../../Common/Data/json/prettywriter.h"
#include "../../Common/Data/json/stringbuffer.h"
#include "../../Common/Riot/RiotAPI.h"

RiotEventList::RiotEventList(rapidjson::Document& document, const std::vector<std::shared_ptr<RiotIngameEvent>>& epicTakedownsKnown, float currentIngameTime) {
	auto allEvents = document["Events"].GetArray();
	std::vector<std::shared_ptr<RiotIngameEvent>> turretEventKills;
	std::vector<std::shared_ptr<RiotIngameEvent>> inhibitorKills;
	std::vector<std::shared_ptr<RiotIngameEvent>> otherEvents;

	turretsDestroyed.insert_or_assign(TeamType::BLUE, 0);
	turretsDestroyed.insert_or_assign(TeamType::RED, 0);
	gameStarted = false;

	for (auto eventIt = allEvents.Begin(); eventIt != allEvents.End(); eventIt++) {
		auto event = eventIt->GetObj();
		const char* eventName = event["EventName"].GetString();
		float eventTime = event["EventTime"].GetFloat();
		if (currentIngameTime < eventTime) {
			continue;
		}
		std::shared_ptr<RiotIngameEvent> newEvent;
		RiotAPI::LogJsonObject(eventIt);
		if (_stricmp(RiotIngameEvent::INHIBITOR_KILLED_EVENT_NAME, eventName) == 0) {
			const char* destroyedInhibitor = event["InhibKilled"].GetString();
			std::string inhibAsString = std::string(destroyedInhibitor);
			size_t inhibitorIdPos = inhibAsString.find("_L");
			if (inhibitorIdPos == std::string::npos) {
				continue;
			}
			InhibitorPosition inhibitorType = static_cast<InhibitorPosition>(atoi(&destroyedInhibitor[inhibitorIdPos + 2]));
			if (inhibitorType < InhibitorPosition::BOTLANE || inhibitorType > InhibitorPosition::TOPLANE) {
				continue;
			}
			uint32_t teamIdNumeric = atoi(&destroyedInhibitor[7]);
			TeamType teamId = static_cast<TeamType>(teamIdNumeric/100 - 1);
			if (teamId < TeamType::BLUE || teamId > TeamType::RED) {
				continue;
			}

			newEvent = std::shared_ptr<RiotIngameEvent>(new InhibitorKilledEvent(eventName, eventTime, Inhibitor(static_cast<uint32_t>(inhibitorType), teamId, inhibitorType, destroyedInhibitor)));
			if (!isDuplicate(inhibitorKills, newEvent)) {
				inhibitorKills.push_back(newEvent);
			}
		}
		else if (_stricmp(RiotIngameEvent::TURRET_KILLED_EVENT_NAME, eventName) == 0) {
			const char* destroyedTurret = event["TurretKilled"].GetString();
			uint32_t turretDestroyedTeamId = (atoi(&destroyedTurret[8])/100) - 1;
			TeamType turretDestroyerTeam = (turretDestroyedTeamId == 0 ? TeamType::RED : TeamType::BLUE);
			newEvent = std::shared_ptr<RiotIngameEvent>(new TurretKilledEvent(eventName, eventTime, destroyedTurret, turretDestroyerTeam));
			if (!isDuplicate(turretEventKills, newEvent)) {
				turretsDestroyed[turretDestroyerTeam]++;
				turretEventKills.push_back(newEvent);
			}
		}
		else {
			if (_stricmp(RiotIngameEvent::GAME_STARTED_EVENT_NAME, eventName) == 0) {
				gameStarted = true;
			}
			newEvent = std::shared_ptr<RiotIngameEvent>(new RiotIngameEvent(eventName, eventTime));
			if (!isDuplicate(otherEvents, newEvent)) {
				otherEvents.push_back(newEvent);
			}
		}
	}
	this->events.insert(std::make_pair(RiotIngameEvent::INHIBITOR_KILLED_EVENT_NAME, inhibitorKills));
	this->events.insert(std::make_pair(RiotIngameEvent::TURRET_KILLED_EVENT_NAME, turretEventKills));
	std::vector<std::shared_ptr<RiotIngameEvent>> drakes, barons, riftHeralds;
	for (auto it : epicTakedownsKnown) {
		auto event = it;
		if (event->getOccuranceTime() <= currentIngameTime) {
			if (_stricmp(RiotIngameEvent::BARON_KILLED_EVENT_NAME, event->getEventName()) == 0) {
				barons.push_back(event);
			}
			else if (_stricmp(RiotIngameEvent::DRAKE_KILLED_EVENT_NAME, event->getEventName()) == 0) {
				const static float MINIMUM_TIME_UNTIL_FIRST_SPAWN = 300.0f;
				if (event->getOccuranceTime() <= MINIMUM_TIME_UNTIL_FIRST_SPAWN) {
					continue;
				}
				drakes.push_back(event);
			}
			else if (_stricmp(RiotIngameEvent::RIFT_HERALD_KILLED_EVENT_NAME, event->getEventName()) == 0) {
				riftHeralds.push_back(event);
			}
		}
	}
	this->events.insert(std::make_pair(RiotIngameEvent::DRAKE_KILLED_EVENT_NAME, drakes));
	this->events.insert(std::make_pair(RiotIngameEvent::BARON_KILLED_EVENT_NAME, barons));
	this->events.insert(std::make_pair(RiotIngameEvent::RIFT_HERALD_KILLED_EVENT_NAME, riftHeralds));
	this->events.insert(std::make_pair("", otherEvents));
}

RiotEventList::~RiotEventList() {
}

bool RiotEventList::isDuplicate(std::vector<std::shared_ptr<RiotIngameEvent>>& currentlyStoredEvents, std::shared_ptr<RiotIngameEvent> event) const {
	for (auto currentEvent : currentlyStoredEvents) {
		if (event->isIdenticalTo(currentEvent.get())) {
			return true;
		}
	}
	return false;
}