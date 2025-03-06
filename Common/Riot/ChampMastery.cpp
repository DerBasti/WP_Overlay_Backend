#include "ChampMastery.h"
#include "../Data/ChampionReader.h"

PlayerChampionMastery::PlayerChampionMastery(std::string summonerPuuid, uint32_t championId, uint64_t masteryAmount) {
	this->summonerPuuid = summonerPuuid;
	this->championId = championId;
	this->champ = ChampionDatabase::getInstance()->getChampionById(championId);
	this->masteryAmount = masteryAmount;
}

PlayerChampionMastery::~PlayerChampionMastery() {

}