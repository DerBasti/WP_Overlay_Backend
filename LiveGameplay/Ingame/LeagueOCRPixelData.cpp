#include "LeagueOCRPixelData.h"


LeagueTeamOCRPixelData::LeagueTeamOCRPixelData(HDC hdc, RECT windowRect, TeamType teamType) {
	this->teamType = teamType;

	money = nullptr;
	grubs = nullptr;

	onWindowResize(hdc, windowRect);
}

LeagueTeamOCRPixelData::~LeagueTeamOCRPixelData() {
	delete money;
	money = nullptr;

	delete grubs;
	grubs = nullptr;
}

void LeagueTeamOCRPixelData::onWindowResize(HDC hdc, RECT windowRect) {
	RECT moneyRect = (teamType == TeamType::BLUE) ? CalculateBlueMoneyRect(windowRect) : CalculateRedMoneyRect(windowRect);
	RECT grubsRect = (teamType == TeamType::BLUE) ? CalculateBlueGrubsRect(windowRect) : CalculateRedGrubsRect(windowRect);

	delete money;
	delete grubs;

	money = new LeagueOCRPixelData(hdc, moneyRect);
	grubs = new LeagueOCRPixelData(hdc, grubsRect);
}

RECT LeagueTeamOCRPixelData::CalculateBlueMoneyRect(RECT windowRect) const {
	int32_t totalWidth = windowRect.right - windowRect.left;
	int32_t totalHeight = windowRect.bottom - windowRect.top;

	float blueGoldXStart = (totalWidth * 0.4175f);
	float blueGoldXEnd = (totalWidth * 0.4475f) - blueGoldXStart;
	float blueGoldYStart = totalHeight * 0.015f;
	float blueGoldYEnd = (totalHeight * 0.035f) - blueGoldYStart;
	RECT blueSideRect = RECT{ (LONG)blueGoldXStart, (LONG)blueGoldYStart, (LONG)blueGoldXEnd, (LONG)blueGoldYEnd };
	return blueSideRect;
}

RECT LeagueTeamOCRPixelData::CalculateRedMoneyRect(RECT windowRect) const {
	int32_t totalWidth = windowRect.right - windowRect.left;
	int32_t totalHeight = windowRect.bottom - windowRect.top;

	float redGoldXStart = (totalWidth * 0.5775f);
	float redGoldXEnd = (totalWidth * 0.6075f) - redGoldXStart;
	float redGoldYStart = totalHeight * 0.015f;
	float redGoldYEnd = (totalHeight * 0.035f) - redGoldYStart;
	RECT redSideRect = RECT{ (LONG)redGoldXStart, (LONG)redGoldYStart, (LONG)redGoldXEnd, (LONG)redGoldYEnd };
	return redSideRect;
}

RECT LeagueTeamOCRPixelData::CalculateBlueGrubsRect(RECT windowRect) const {
	int32_t totalWidth = windowRect.right - windowRect.left;
	int32_t totalHeight = windowRect.bottom - windowRect.top;

	float blueGrubsXStart = (totalWidth * 0.3453f);
	float blueGrubsXEnd = (totalWidth * 0.3753f) - blueGrubsXStart;
	float blueGrubsYStart = totalHeight * 0.017f;
	float blueGrubsYEnd = (totalHeight * 0.035f) - blueGrubsYStart;
	RECT blueSideRect = RECT{ (LONG)blueGrubsXStart, (LONG)blueGrubsYStart, (LONG)blueGrubsXEnd, (LONG)blueGrubsYEnd };
	return blueSideRect;
}

RECT LeagueTeamOCRPixelData::CalculateRedGrubsRect(RECT windowRect) const {
	int32_t totalWidth = windowRect.right - windowRect.left;
	int32_t totalHeight = windowRect.bottom - windowRect.top;

	float redGoldXStart = (totalWidth * 0.65f);
	float redGoldXEnd = (totalWidth * 0.68f) - redGoldXStart;
	float redGrubsYStart = totalHeight * 0.017f;
	float redGrubsYEnd = (totalHeight * 0.035f) - redGrubsYStart;
	RECT redSideRect = RECT{ (LONG)redGoldXStart, (LONG)redGrubsYStart, (LONG)redGoldXEnd, (LONG)redGrubsYEnd };
	return redSideRect;
}