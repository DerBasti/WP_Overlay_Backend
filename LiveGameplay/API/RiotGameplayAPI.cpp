#include "RiotGameplayAPI.h"
#include "RiotEventList.h"
#include "../../Common/Data/CURLWrapper.h"
#include "../../Common/Riot/RiotAPI.h"

using namespace rapidjson;


RiotGameplayAPI::RiotGameplayAPI() : RiotIngameAPI() {
	startedFlag = false;
	/*liveApiInstance.addObjectTakenDownListener([&](std::shared_ptr<RiotIngameEvent> event) {
		bool foundDuplicate = false;
		std::lock_guard<std::mutex> lock(epicTakedownEventsMutex);
		for (auto epicBuff : epicBuffTakedownEvents) {
			if (epicBuff->isIdenticalTo(event.get())) {
				foundDuplicate = true;
				break;
			}
		}
		if (!foundDuplicate) {
			epicBuffTakedownEvents.push_back(event);
		}
	});
	*/
}

RiotGameplayAPI::~RiotGameplayAPI() {
	//stopLiveApi();
}

#include "../../Common/PerformanceClock.h"
std::shared_ptr<RiotPlayerList> RiotGameplayAPI::getPlayerOverview() {
	PerformanceClock clock;
	dumpNextResponseBody();
	Document playerJson = getRequestToApi("liveclientdata/playerlist");
	auto duration = clock.getDuration();
	auto jsonType = playerJson.GetType();
	if (playerJson.HasParseError() || playerJson.IsNull()) {
		return std::shared_ptr<RiotPlayerList>();
	}
	return std::shared_ptr<RiotPlayerList>(new RiotPlayerList(playerJson, getCurrentGametime()));
}

std::shared_ptr<RiotEventList> RiotGameplayAPI::getOccuredEvents() {
	Document events = getRequestToApi("liveclientdata/eventdata");
	if (events.HasParseError() || !events.IsObject()) {
		return std::shared_ptr<RiotEventList>();
	}
	std::lock_guard<std::mutex> lock(epicTakedownEventsMutex);
	RiotEventList *list = new RiotEventList(events, epicBuffTakedownEvents, getCurrentGametime());
	return std::shared_ptr<RiotEventList>(list);
}

float RiotGameplayAPI::getCurrentGametime() {
	Document gameinfoJson = getRequestToApi("liveclientdata/gamestats");
	if (!gameinfoJson.HasParseError() && !gameinfoJson.IsNull()) {
		float result = gameinfoJson["gameTime"].GetFloat();
		//liveApiInstance.updateCurrentIngameTime(result);
		return result;
	}
	return -1.0f;
}

void RiotGameplayAPI::readReplayData() {
	Document gameinfoJson = getRequestToApi("replay/playback");
	if (!gameinfoJson.HasParseError() && !gameinfoJson.IsNull()) {
		RiotAPI::LogJsonObject(&gameinfoJson);
	} 
}

void RiotGameplayAPI::subscribeToEvents() {
	postRequestToApi("Subscribe", "eventName=SpectatorUpdate");
}

void RiotGameplayAPI::esportsObserverReady() {
	Document gameinfoJson = getRequestToApi("esportsObserverReady");
	if (!gameinfoJson.HasParseError() && !gameinfoJson.IsNull()) {
		RiotAPI::LogJsonObject(&gameinfoJson);
	}
}