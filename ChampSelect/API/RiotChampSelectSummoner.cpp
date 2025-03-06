#include "RiotChampSelectSummoner.h"

ChampSelectSummoner::ChampSelectSummoner(std::wstring botName, uint8_t slotId) {
	this->summonerId = 0;
	this->name = botName;
	this->wasActing = false;
	teamType = TeamType::INVALID;
	currentAction = ChampSelectSummonerActionType::IDLE;
	this->currentlyPickedChampionId = 0;
	this->finishedPicking = false;
	this->slotId = slotId;
}
ChampSelectSummoner::ChampSelectSummoner(std::shared_ptr<RiotSummoner> summoner, uint8_t slotId) {
	this->summonerId = summoner ? summoner->getSummonerId() : 0;
	this->name = summoner ? summoner->getAccountNameEncoded() : L"";
	this->wasActing = false;
	teamType = TeamType::INVALID;
	currentAction = ChampSelectSummonerActionType::IDLE;
	this->currentlyPickedChampionId = 0;
	this->puuid = summoner ? summoner->getPuuid() : "";
	this->finishedPicking = false;
	this->slotId = slotId;
}

ChampSelectSummoner::~ChampSelectSummoner() {

}