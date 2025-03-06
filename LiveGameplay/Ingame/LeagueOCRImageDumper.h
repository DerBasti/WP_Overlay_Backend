#ifndef __LEAGUE_OCR_IMAGE_DUMPER__
#define __LEAGUE_OCR_IMAGE_DUMPER__

#pragma once 

#include "LeagueOCRBase.h"
#include "../../Common/Data/Team.h"
#include "LeagueMoneyOCRBoundingBox.h"
#include <Windows.h>
#include <memory>

class LeagueOCRPixelData;

enum class LeagueOCRImageDumpType : uint8_t {
	PLAIN,
	WITH_BOUNDING_BOXES_ONLY,
	WITH_FILL_ONLY,
	WITH_BOUNDING_BOX_AND_FILL
};

class LeagueOCRImageDumper {
private:
	TeamType teamType;
	const LeagueOCRPixelData* pixelData;

	void spanScan();
	void quadScan();
	void spanFill(LeagueOCRPixelData* pixelData, std::pair<uint32_t, uint32_t> firstValidPixel) const;
	void quadFill();

	void drawBoundingBox(LeagueOCRPixelData* pixelData, const LetterBoundingBox* box) const;

	void drawColorAtPosition(LeagueOCRPixelData* pixelData, uint32_t color, uint32_t x, uint32_t y) const;

	HANDLE createAndPrepareOutputFile(const wchar_t* outputPath, std::shared_ptr<BITMAPINFO> bitmapInfo) const;
	std::shared_ptr<BITMAPINFO> createBitmapInfoStruct(HBITMAP hBmp) const;
public:
	LeagueOCRImageDumper(const LeagueOCRPixelData* pixelData, TeamType teamType);
	LeagueOCRImageDumper(const LeagueOCRImageDumper&) = delete;
	virtual ~LeagueOCRImageDumper();

	bool dumpImage(const wchar_t* outputPath, LeagueOCRImageDumpType dumpType) const;
};

#endif //__LEAGUE_OCR_IMAGE_DUMPER__