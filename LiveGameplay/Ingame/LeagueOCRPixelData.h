#ifndef __LEAGUE_OCR_PIXEL_DATA__
#define __LEAGUE_OCR_PIXEL_DATA__

#include <Windows.h>
#include <inttypes.h>
#include "../../Common/Data/Team.h"

class LeagueOCRPixelData {
private:
	HBITMAP bitmap;
	uint8_t* pixelMemory;
	RECT ocrRect;
public:
	LeagueOCRPixelData(const LeagueOCRPixelData&) = delete;
	LeagueOCRPixelData(uint8_t* copyPixelData, RECT ocrRect) {
		pixelMemory = copyPixelData;
		this->ocrRect = ocrRect;
		bitmap = nullptr;
	}
	LeagueOCRPixelData(HDC hdc, RECT ocrRect) {
		this->ocrRect = ocrRect;
		pixelMemory = new uint8_t[ocrRect.right * ocrRect.bottom * sizeof(uint32_t)];
		bitmap = CreateCompatibleBitmap(hdc, ocrRect.right, ocrRect.bottom);
	}
	virtual ~LeagueOCRPixelData() {
		DeleteObject(bitmap);
		bitmap = nullptr;

		delete[] pixelMemory;
		pixelMemory = nullptr;
		ocrRect = { 0 };
	}
	inline uint32_t getWidth() const {
		return ocrRect.right;
	}
	inline uint32_t getHeight() const {
		return ocrRect.bottom;
	}
	inline RECT getOCRRect() const {
		return ocrRect;
	}
	inline HBITMAP getHBitmap() const {
		return bitmap;
	}
	inline uint8_t* getPixelMemory() const {
		return pixelMemory;
	}
	inline uint32_t* getPixelMemoryAsSinglechannel() const {
		return (uint32_t*)pixelMemory;
	}
};

class LeagueTeamOCRPixelData {
private:
	LeagueOCRPixelData* money;
	LeagueOCRPixelData* grubs;
	TeamType teamType;

	RECT CalculateBlueMoneyRect(RECT windowRect) const;
	RECT CalculateRedMoneyRect(RECT windowRect) const;

	RECT CalculateBlueGrubsRect(RECT windowRect) const;
	RECT CalculateRedGrubsRect(RECT windowRect) const;
public:
	LeagueTeamOCRPixelData(HDC hdc, RECT windowRect, TeamType teamType);
	LeagueTeamOCRPixelData(const LeagueTeamOCRPixelData&) = delete;
	virtual ~LeagueTeamOCRPixelData();
	void onWindowResize(HDC hdc, RECT windowRect);

	inline LeagueOCRPixelData* getMoneyOCRPixelData() const {
		return money;
	}
	inline LeagueOCRPixelData* getGrubsOCRPixelData() const {
		return grubs;
	}
	inline TeamType getTeamType() const {
		return teamType;
	}
};

#endif //