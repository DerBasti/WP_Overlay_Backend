
#include "../GUI/GUIColor.h"
#include "RiotRankTiers.h"


/*
	UNRANKED,
	IRON,
	BRONZE,
	SILVER,
	GOLD,
	PLATINUM,
	EMERALD,
	DIAMOND,
	MASTER,
	GRANDMASTER,
	CHALLENGER,
	UNKNOWN
*/
const GUIColor RiotRanking::RankColors[] = {
	GUIColor::WHITE,
	GUIColor::IRON_RANK,
	GUIColor::BRONZE_RANK,
	GUIColor::SILVER_RANK,
	GUIColor::GOLD_RANK,
	GUIColor::PLATIN_RANK,
	GUIColor::EMERALD_RANK,
	GUIColor::DIAMOND_RANK,
	GUIColor::MASTER_RANK,
	GUIColor::GRANDMASTER_RANK,
	GUIColor::CHALLENGER_RANK,
	GUIColor::WHITE
};

RiotRanking::RiotRanking() : RiotRanking(RankedTier::UNRANKED, 0, 0) {}

RiotRanking::RiotRanking(RankedTier tier, uint8_t div, int16_t lp) {
	this->tier = tier;
	this->division = div;
	this->lp = lp;
}
RiotRanking::~RiotRanking() {

}
uint8_t RiotRanking::FromStringToDivision(const char* str) {
	static std::unordered_map<std::string, uint8_t> DIVISIONS = {
		{"", 0},
		{"NA", 0},
		{"I", 1},
		{"II", 2},
		{"III", 3},
		{"IV", 4},
		{"V", 5},
	};
	return DIVISIONS.at(str);
}


std::wstring RiotRanking::ToStringFromRankedTier(RankedTier rankedTier) {
	static std::unordered_map<RankedTier, std::wstring> map{
		{ RankedTier::UNRANKED, L"Unranked"},
		{ RankedTier::IRON, L"Iron" },
		{ RankedTier::BRONZE, L"Bronze" },
		{ RankedTier::SILVER, L"Silver" },
		{ RankedTier::GOLD, L"Gold" },
		{ RankedTier::PLATINUM, L"Platinum" },
		{ RankedTier::EMERALD, L"Emerald" },
		{ RankedTier::DIAMOND, L"Diamond" },
		{ RankedTier::MASTER, L"Master" },
		{ RankedTier::GRANDMASTER, L"Grandmaster" },
		{ RankedTier::CHALLENGER, L"Challenger" },
		{ RankedTier::UNKNOWN, L"Unranked" },
	};
	return map.at(rankedTier);
}

std::wstring RiotRanking::toUnicodeString() const {
	std::wstring rankedTier = ToStringFromRankedTier(getTier());
	wchar_t buffer[0x80] = { 0x00 };
	if (getTier() == RankedTier::UNRANKED) {
		swprintf_s(buffer, rankedTier.c_str());
	}
	else if (getTier() < RankedTier::MASTER) {
		swprintf_s(buffer, L"%s%i (%iLP)", rankedTier.c_str(), getDivision(), getLp());
	}
	else {
		swprintf_s(buffer, L"%s (%iLP)", rankedTier.c_str(), getLp());
	}
	return std::wstring(buffer);
}