#ifndef __LEAGUE_RIOT_EVENT_LIST__
#define __LEAGUE_RIOT_EVENT_LIST__

#include "../../Common/Data/json/document.h"
#include <unordered_map>
#include <vector>
#include "../../Common/Logging/Logger.h"
#include "../../Common/Data/Team.h"
#include "Event.h"

class RiotEventList {
private:
	ROSELogger logger;
	bool gameStarted;
	std::unordered_map<const char*, std::vector<std::shared_ptr<RiotIngameEvent>>> events;
	std::unordered_map<TeamType, uint32_t> turretsDestroyed;
	std::unordered_map<TeamType, uint32_t> teamKills;
	bool isDuplicate(std::vector<std::shared_ptr<RiotIngameEvent>>& currentlyStoredEvents, std::shared_ptr<RiotIngameEvent> event) const;
public:
	RiotEventList() = default;
	RiotEventList(rapidjson::Document& document, const std::vector<std::shared_ptr<RiotIngameEvent>>& epicTakedownsKnown, float currentIngameTime);
	virtual ~RiotEventList();

	__inline const std::vector<std::shared_ptr<RiotIngameEvent>>& getEventsByName(const char* eventName) const {
		return events.at(eventName);
	}
	__inline bool hasGameStarted() const {
		return gameStarted;
	}
	inline uint32_t getTurretsDestroyedByTeam(TeamType type) const {
		return turretsDestroyed.at(type);
	}
};

#endif //__LEAGUE_RIOT_EVENT_LIST__