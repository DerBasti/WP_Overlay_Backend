#include "RiotPlayerList.h"

RiotPlayerList::RiotPlayerList(rapidjson::Document& document, float gametime) {
	uint8_t currentSlot = 0;
	this->blueTeam = new Team(TeamType::BLUE);
	this->redTeam = new Team(TeamType::RED);
	for (auto it = document.Begin(); it != document.End(); it++) {
		auto playerEntry = it->GetObj();
		IngamePlayerSnapshot* snapshot = new IngamePlayerSnapshot(playerEntry, currentSlot);
		if (currentSlot <= 4) {
			blueTeam->addChampionSnapshot(snapshot);
		}
		else {
			redTeam->addChampionSnapshot(snapshot);
		}
		currentSlot++;
	}
	this->gametime = gametime;
}

RiotPlayerList::~RiotPlayerList() {
	delete blueTeam;
	delete redTeam;
}