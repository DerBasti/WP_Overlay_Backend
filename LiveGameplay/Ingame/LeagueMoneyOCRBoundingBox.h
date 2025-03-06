#ifndef __LEAGUE_MONEY_OCR_BOUNDING_BOX__
#define __LEAGUE_MONEY_OCR_BOUNDING_BOX__

#pragma once

#include "../../Common/Logging/Logger.h"
#include "LeagueOCRBase.h"
#include "../../Common/GUI/GUIColor.h"
#include "LeagueOCRPixelData.h"
#include <Windows.h>

class LeagueOCRPixelData;

class LetterBoundingBox {
private:
	uint32_t* pixel;
	uint32_t width;
	uint32_t height;
	RECT boundingBox;
	LeagueOCRLocaleDetectionMode detectionMode;
	LetterBoundingBoxColor dominantColor;

	friend class LetterBoundingBoxScanner;

	inline uint32_t translateBitmapToActualYCoordinate(uint32_t y, uint32_t height) const {
		return (height - y - 1);
	}
public:
	LetterBoundingBox(const LeagueOCRPixelData* pixelData, LetterBoundingBoxColor dominantColor);
	virtual ~LetterBoundingBox();

	inline void setDetectionMode(LeagueOCRLocaleDetectionMode detectionMode) {
		this->detectionMode = detectionMode;
	}

	inline void overrideX(uint32_t x) {
		boundingBox.right -= ((LONG)x - boundingBox.left);
		boundingBox.left = x;
	}
	inline RECT getBoundingBox() const {
		return boundingBox;
	}
	inline RECT getBoundingBoxConvertedCoordinates() const {
		return RECT{ boundingBox.left,
			(LONG)translateBitmapToActualYCoordinate(boundingBox.top, height),
			boundingBox.right,
			(LONG)translateBitmapToActualYCoordinate(boundingBox.bottom, height) };
	}
	inline uint32_t getBoundingWidth() const {
		return boundingBox.right - boundingBox.left;
	}
	inline uint32_t getBoundingHeight() const {
		return boundingBox.bottom - boundingBox.top;
	}
	inline bool isValid() const {
		return boundingBox.left >= 0 && boundingBox.top >= 0 && boundingBox.right >= 1 && boundingBox.bottom >= 1;
	}
};

class LetterBoundingBoxScanner {
private:
	const LeagueOCRPixelData* pixelData;
	LetterBoundingBoxColor dominantColor;

	LetterBoundingBox startRayCasting(LeagueOCRLocaleDetectionMode detectionMode, uint32_t xStart);

	bool isValidColor(uint32_t color, uint32_t colorValue) const;
	bool isValidColorAtPosition(uint32_t x, uint32_t y);
	void castRightRayFromLower(LetterBoundingBox* boundingBox);
	void castRightRayFromUpper(LetterBoundingBox* boundingBox);
	void castUpperLeftRayFromLower(LetterBoundingBox* boundingBox, uint32_t xStart);
	void castUpperRightRayFromLower(LetterBoundingBox* boundingBox);

	bool castCenterRay(LetterBoundingBox* boundingBox, uint32_t xStart) const;
	void rayFill(LetterBoundingBox* letterBox, uint32_t xStart);
	void spanFill(LetterBoundingBox* letterBox);
	void quadFill(LetterBoundingBox* letterBox);

	void findBoundingBoxLower(LetterBoundingBox* boundingBox, uint32_t xStart);
	void castLowerLeftRayFromUpper(LetterBoundingBox* boundingBox, uint32_t xStart);
	bool isColorSomehowConnectedLeftside(uint32_t currentX, uint32_t currentY);
	bool isColorSomehowConnectedRightside(uint32_t currentX, uint32_t currentY);
	void findBoundingBoxUpper(LetterBoundingBox* boundingBox, uint32_t xStart);
	void findBoundingBoxCenter(LetterBoundingBox* boundingBox, uint32_t xStart);

	static std::pair<GUIColor, uint32_t> ExtractPixelColorAndValueFromCoordinate(const LeagueOCRPixelData* pixel, uint32_t x, uint32_t y);
	std::pair<GUIColor, uint32_t> extractPixelColorAndValueFromCoordinate(uint32_t x, uint32_t y) const;

	static inline uint32_t TranslateBitmapToActualYCoordinate(uint32_t y, uint32_t height) {
		return (height - y - 1);
	}
	inline uint32_t getPixelColor(uint32_t x, uint32_t y) const {
		return GetPixelColor(pixelData->getPixelMemoryAsSinglechannel(), x, y, pixelData->getWidth());
	}
	static inline uint32_t GetPixelColor(const LeagueOCRPixelData* pixel, uint32_t x, uint32_t y) {
		return GetPixelColor(pixel->getPixelMemoryAsSinglechannel(), x, y, pixel->getWidth());
	}
	static inline uint32_t GetPixelColor(uint32_t* pixel, uint32_t x, uint32_t y, uint32_t width) {
		return pixel[y * width + x] & 0xFFFFFF;
	}

	inline uint32_t extractPixelValue(uint32_t x, uint32_t y) const {
		uint32_t currentPixel = getPixelColor(x, y);
		return ExtractPixelValue(currentPixel);
	}

	static inline uint32_t ExtractPixelValue(uint32_t currentPixelColor) {
		return ((currentPixelColor >> 16) & 0xFF) + ((currentPixelColor >> 8) & 0xFF) + (currentPixelColor & 0xFF);
	}
public:
	LetterBoundingBoxScanner(const LeagueOCRPixelData* pixelData, LetterBoundingBoxColor dominantColor);
	LetterBoundingBoxScanner(const LetterBoundingBoxScanner&) = delete;
	virtual ~LetterBoundingBoxScanner();

	std::pair<uint32_t, uint32_t> findFirstValidPixelInsideBoundingBox(const LetterBoundingBox* box) const;

	std::vector<LetterBoundingBox> detectBoundingBoxes(LeagueOCRLocaleDetectionMode detectionMode);

	static bool IsValidColor(uint32_t color, uint32_t colorValue, LetterBoundingBoxColor dominantColorToCheckFor);
	static bool IsValidColorAtPosition(const LeagueOCRPixelData* pixel, LetterBoundingBoxColor dominantColorToCheckFor, uint32_t x, uint32_t y);
};

#endif //__LEAGUE_MONEY_OCR_BOUNDING_BOX__