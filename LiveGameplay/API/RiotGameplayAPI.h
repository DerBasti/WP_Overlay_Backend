#ifndef __LEAGUE_RIOT_GAMEPLAY_API__
#define __LEAGUE_RIOT_GAMEPLAY_API__

#include "../../Common/Data/json/document.h"
#include "RiotPlayerList.h"
#include "RiotEventList.h"
//#include "RiotLiveAPI.h"
#include "../../Common/Riot/RiotIngameAPI.h"

class RiotGameplayAPI : public RiotIngameAPI {
private:
	bool startedFlag;
	//RiotLiveAPI liveApiInstance;
	std::mutex epicTakedownEventsMutex;
	std::vector<std::shared_ptr<RiotIngameEvent>> epicBuffTakedownEvents;
public:
	RiotGameplayAPI();
	virtual ~RiotGameplayAPI();
	std::shared_ptr<RiotPlayerList> getPlayerOverview();
	float getCurrentGametime();
	std::shared_ptr<RiotEventList> getOccuredEvents();
	void readReplayData();
	void subscribeToEvents();
	void esportsObserverReady();
};

#endif //__LEAGUE_RIOT_GAMEPLAY_API__