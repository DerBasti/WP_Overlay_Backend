#include "ChampionSnapshot.h"
#include "../../Common/Data/ItemDatabase.h"
#include "../../Common/Data/ChampionReader.h"
#include "../../Common/Data/SummonerSpell.h"
#include "../../Common/Riot/RiotMatchStatsNames.h"

template<>
struct std::less<Item*>
{	
	constexpr bool operator()(const Item* _Left, const Item* _Right) const
	{
		return _Right == nullptr || (_Left != nullptr && _Right != nullptr && _Left->getId() < _Right->getId());
	}
};


IngamePlayerSnapshot::IngamePlayerSnapshot(rapidjson::GenericObject<false, rapidjson::Value>& playerEntry, uint8_t spectatorSlotId) {
	this->champion = ChampionDatabase::getInstance()->getChampionByName(playerEntry["championName"].GetString());

	std::string summonerName = std::string(playerEntry["summonerName"].GetString());
	int len = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, summonerName.c_str(), (int)summonerName.length(), NULL, 0);
	std::wstring wstr(len, 0);
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, summonerName.c_str(), (int)summonerName.length(), &wstr[0], len);
	this->summonerName = wstr;

	auto items = playerEntry["items"].GetArray();
	for (uint8_t i = 0; i < inventory.max_size(); i++) {
		inventory[i] = nullptr;
		itemActivatable[i] = true;
	}
	this->level = playerEntry["level"].GetInt();
	std::vector<Item*> nonEmptyItems = {};
	std::map<uint32_t, bool> value;
	for (auto itemIt = items.Begin(); itemIt != items.End(); itemIt++) {
		auto itemJson = itemIt->GetObj();
		int itemId = itemJson["itemID"].GetInt();
		Item *item = ItemDatabase::getInstance()->getItem(itemId);
		if (item != nullptr) {
			value.insert_or_assign(itemId, itemJson["canUse"].GetBool());
			nonEmptyItems.push_back(item);
		}
	}
	std::sort(nonEmptyItems.begin(), nonEmptyItems.end(), std::less<Item*>());
	for (uint32_t i = 0; i < inventory.max_size(); i++) {
		inventory[i] = (i < nonEmptyItems.size()) ? nonEmptyItems.at(i) : nullptr;
		if (inventory[i]) {
			this->itemActivatable[i] = value.at(inventory[i]->getId());
		}
	}

	firstSummonerSpell = nullptr;
	secondSummonerSpell = nullptr;

	auto summonerSpells = playerEntry["summonerSpells"].GetObj();
	if (summonerSpells.HasMember("summonerSpellOne")) {
		auto summonerSpellName = summonerSpells["summonerSpellOne"].GetObj()["displayName"].GetString();
		firstSummonerSpell = SummonerSpell::GetSpellByName(summonerSpellName);
	}
	if (summonerSpells.HasMember("summonerSpellTwo")) {
		auto summonerSpellName = summonerSpells["summonerSpellTwo"].GetObj()["displayName"].GetString();
		secondSummonerSpell = SummonerSpell::GetSpellByName(summonerSpellName);
	}

	auto scores = playerEntry[RiotIngameStatNames::PARENT_NODE_NAME].GetObj();

	this->creepsKilled = scores[RiotIngameStatNames::CS_AMOUNT].GetInt();// +scores[RiotPlayerStatsNames::NEUTRAL_MINIONS_AMOUNT].GetInt();
	this->champsKilled = scores[RiotIngameStatNames::KILLS].GetInt();
	this->deaths = scores[RiotIngameStatNames::DEATHS].GetInt();
	this->killAssists = scores[RiotIngameStatNames::ASSISTS].GetInt();
	this->respawnTimer = playerEntry["respawnTimer"].GetFloat();
	this->spectatorSlotId = spectatorSlotId;
	this->team = _stricmp(playerEntry["team"].GetString(), "ORDER") == 0 ? TeamType::BLUE : TeamType::RED;
}

IngamePlayerSnapshot::~IngamePlayerSnapshot() {
	for (uint8_t i = 0; i < inventory.max_size(); i++) {
		inventory[i] = nullptr;
	}
	team = TeamType::INVALID;
}

std::string IngamePlayerSnapshot::getChampionName() const {
	return (champion != nullptr ? champion->getIngameName() : std::string());
}

bool IngamePlayerSnapshot::doesInventoryItemExist(uint32_t id) const {
	for (auto item : getInventory()) {
		if (item == nullptr) {
			continue;
		}
		if (item->getId() == id) {
			return true;
		}
	}
	return false;
}