#ifndef __RIOT_CHAMP_MASTERY__
#define __RIOT_CHAMP_MASTERY__

#include <inttypes.h>
#include <string>

class Champion;

class PlayerChampionMastery {
private:
	Champion* champ;
	uint32_t championId;
	std::string summonerPuuid;
	uint64_t masteryAmount;
public:
	PlayerChampionMastery() : PlayerChampionMastery("", 0, 0) {

	}
	PlayerChampionMastery(std::string summonerPuuid, uint32_t championId, uint64_t masteryAmount);
	virtual ~PlayerChampionMastery();
	inline Champion* getChampion() const {
		return champ;
	}
	inline uint32_t getChampionId() const {
		return championId;
	}
	inline std::string getSummonerPuuid() const {
		return summonerPuuid;
	}
	inline uint64_t getMasteryAmount() const {
		return masteryAmount;
	}
};

#endif //__RIOT_CHAMP_MASTERY__