#ifndef __LEAGUE_RIOT_PLAYER_LIST__
#define __LEAGUE_RIOT_PLAYER_LIST__

#include "../../Common/Data/json/document.h"
#include "../Ingame/ChampionSnapshot.h"

class RiotPlayerList {
private:
	Team* blueTeam;
	Team* redTeam;
	float gametime;
public:
	RiotPlayerList(rapidjson::Document& document, float gametime);
	virtual ~RiotPlayerList();

	__inline float getGametime() const {
		return gametime;
	}
	
	__inline Team* getBlueTeam() const {
		return blueTeam;
	}
	
	__inline Team* getRedTeam() const {
		return redTeam;
	}

	__inline size_t getTotalPlayerAmount() const {
		return (blueTeam ? blueTeam->getPlayerAmount() : 0) + (redTeam ? redTeam->getPlayerAmount() : 0);
	}
};

#endif