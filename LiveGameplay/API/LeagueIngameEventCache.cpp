#include "LeagueIngameEventCache.h"

LeagueIngameEventCache::LeagueIngameEventCache() {
	dragonKillTimes.reserve(10);
}

LeagueIngameEventCache::~LeagueIngameEventCache() {

}

bool LeagueIngameEventCache::addKillTiming(EpicMonsterType monsterType, float ingameTime, float respawnTimeBetweenKills) {
	auto& killTimesVector = getEpicKillTimeVector(monsterType);
	for (float timepoint : killTimesVector) {
		float diff = fabsf(timepoint - ingameTime);
		if (diff < respawnTimeBetweenKills) {
			return false;
		}
	}
	killTimesVector.push_back(ingameTime);
	return true;
}

float LeagueIngameEventCache::getLastKillTime(EpicMonsterType monsterType, float currentIngameTime) const {
	auto& killTimesVector = getEpicKillTimeVector(monsterType);
	float lastKillTimepoint = 0.0f;
	for (float timepoint : killTimesVector) {
		if (timepoint > currentIngameTime) {
			break;
		}
		lastKillTimepoint = timepoint;
	}
	return 0.0f;
}